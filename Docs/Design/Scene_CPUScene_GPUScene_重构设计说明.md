# Scene / CPUScene / GPUScene 重构设计说明

## 1. 背景与目标

当前渲染重构目标是把场景语义更新与 GPU 上传解耦，形成稳定的数据流：

1. `Scene` 负责收集组件层脏事件（Transform/Mesh/Material/Create/Destroy）。
2. `CPUScene` 负责把事件沉淀为扁平真值数据（SoA）。
3. `GPUScene` 负责按脏节点增量上传到 GPU（PerObject/AABB/RenderProxy）。

该设计用于同时支持：

1. Legacy 渲染路径（CPU Culling + 普通渲染）。
2. GPUScene 路径（GPU Culling + Indirect Draw）。

---

## 2. 组件与场景职责

### 2.1 资源与组件层

1. `ResourceManager` 负责加载/管理资源（Mesh/Material/Texture/Shader 等）。
2. `MeshFilter` 持有 `Mesh` 引用（`ResourceHandle<Mesh>`）。
3. `MeshRenderer` 持有 `Material` 引用（共享材质/实例材质）。

组件资源变化时只发语义脏标记，不直接做 GPU 上传。

### 2.2 Scene 层

`Scene` 持有所有 `GameObject`，每帧执行：

1. `RunLogicUpdate()`：脚本和逻辑更新。
2. `RunTransformUpdate()`：基于脏根节点递归刷新 transform。
3. 将 transform/material/mesh/create/destroy 变化写入 NodeChange Queue。

Queue 规则：

1. 以 `frameID + renderID` 去重，同帧同节点只入队一次。
2. 同帧重复变化只更新 payload 与 flags，不重复入队。
3. 帧末输出 `dirtyRenderID + dirtyFlags + payload` 给 `RenderEngine`。

---

## 3. RenderEngine 数据消费流程

每帧顺序：

1. `Scene -> CPUScene`：消费 NodeChange Queue，更新 CPU 真值。
2. `CPUScene -> GPUScene`：GPUScene 按 dirty renderID + CPUSceneView 更新 GPU shadow。
3. `GPUScene` 生成每类目标 Buffer 的 CopyOp，并提交拷贝命令。

关键原则：

1. Scene flags 是“来源语义”。
2. GPU 上传按 Buffer 类型组织，不要求和 Scene flags 一一同构。

---

## 4. CPUScene 设计

### 4.1 CPU 真值（SoA）

CPUScene 保存渲染真值（按 renderID 索引）：

1. `meshID`
2. `materialID`
3. `objectToWorld`
4. `localAABB`
5. `worldAABB`
6. `layerMask`

### 4.2 Batch/Proxy 相关

CPUScene 负责 batch 注册计数（Add/Decrease）的语义正确性：

1. `Create`：Add。
2. `Destroy`：Decrease。
3. `Mesh/Material/Layer` 变化：先 Decrease(old) 再 Add(new)。
4. `TransformDirty`：不动 batch 计数。

避免重复注册导致 BatchMap 爆炸。

### 4.3 对 GPU 暴露视图

`CPUSceneView` 仅提供只读访问，不做 GPU 资源操作。

建议用 `const vector&`（优先）或 `const vector*`（可行但易误用）。

---

## 5. GPUScene 设计

### 5.1 GPUScene 输入

GPUScene 输入为：

1. 脏 `renderID` 列表。
2. 对应 flags（可直接使用 Scene 语义，或在 CPUScene 映射后传入）。
3. `CPUSceneView`（读取真值）。

### 5.2 GPUScene CPU-side shadow

GPUScene 在 CPU 端维护 `perObjectDataBuffer` 作为 shadow，用于组装完整 `PerObjectData` 行。

### 5.3 Buffer 更新策略

1. `allObjectDataBuffer`：按 row 覆盖（固定槽位，不 Free）。
2. `allAABBBuffer`：按 row 覆盖（固定槽位，不 Free）。
3. `renderProxyBuffer`：可变长分配，`ProxyDirty` 时重建块（需要 Free 旧块）。
4. staging 使用 `perFrameUploadBuffer`（Upload），目标资源使用 `Default`。

### 5.4 Copy 组织

每帧按目标 buffer 收集 CopyOp：

1. `copyOpsObject`
2. `copyOpsAABB`
3. `copyOpsProxy`

统一提交 `CopyBufferRegion`，而不是每节点立刻提交。

---

## 6. Destroy / 失效策略

### 6.1 CPU 侧

Destroy 时 CPUScene 将节点真值置无效：

1. `meshID/materialID` 置 invalid。
2. `objectToWorld` 置 identity。
3. `AABB` 清空。

### 6.2 GPU 侧

Destroy 时 GPUScene 必须显式写失效，而不是“什么都不做”：

1. `PerObjectData.renderProxyCount = 0`
2. `renderProxyStartIndex = 0`
3. 对应 AABB 行写空
4. 旧 proxy allocation 回收（单帧模式可立即；多帧模式 deferred free）

### 6.3 Shader 侧

`GPUCulling.hlsl` 必须补失效判断：

1. 读取 `PerObjectData` 后，若 `renderProxyCount == 0` 直接 `return`。
2. 支持遍历 `renderProxyCount`，当前对象可对应多个 proxy。

---

## 7. 多帧与 FrameContext（待实现）

> 本节是后续工作重点。

仅有 staging 分帧不够，必须解决“目标 buffer 跨帧读写同址”问题。

### 7.1 风险

多帧并行时，如果 Frame N 仍在读取 `allObjectDataBuffer[x]`，Frame N+1 覆盖同址会产生 hazard。

### 7.2 方案

按 FrameContext 隔离动态目标资源：

1. 每帧各自 `object/aabb/proxy` default buffer（或等价隔离方案）。
2. 每帧各自 upload/copy 列表。
3. 每帧提交 fence，复用该 frame slot 前等待 fence 完成。

### 7.3 Deferred Free

对可变长分配（主要是 proxy）使用 deferred free：

1. 释放请求挂到提交帧上下文。
2. fence 完成后再调用 allocator->Free。

`GPUBufferAllocator` 本身无需改接口，先在上层 FrameContext 管理回收时机即可。

---

## 8. 当前阶段落地清单

1. 打通 `Scene -> CPUScene -> GPUScene` dirty 消费链路。
2. GPUScene 统一使用 per-frame copy 机制，不做散落的即时上传。
3. `GPUCulling.hlsl` 增加 `renderProxyCount==0` 判空与多 proxy 循环。
4. 完善 Destroy 路径：CPU 真值清空 + GPU 行清空 + proxy 回收。
5. 统一 `renderProxyBuffer` 为 `Default`，proxy 更新改为 staging + copy。
6. 后续接入 FrameContext（三帧资源隔离 + deferred free + fence）。

---

## 9. 备注

1. 该方案优先保证数据语义正确性，再做性能优化。
2. 多帧并行前，可先在单帧安全模式验证逻辑闭环。
3. 文档中的命名可按代码实际重命名，但职责边界建议保持不变。
