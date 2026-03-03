# TODO_Update_RenderSync.md

目标：先把 Update / RenderSync 的职责和时序梳理清楚，消除“逻辑更新不清晰”的核心问题。3‑Frame 设计先不展开，实现上只做时序和职责拆分，保持功能不变。

一、现状不适点（核心）

- Game::Update() 里 Scene 更新先于 Resource finalize，导致资源完成时间与组件逻辑交织，时序不稳定。
- Scene::Update() 同时承担逻辑、Transform、render dirty 记录以及 renderSceneData 的同步，职责混杂。
- GPUScene::Tick() 在 RenderPath 里执行，RenderPath 既更新数据又执行渲染，边界不清晰。
- Legacy / GPUScene 的概念被混用：Legacy 实际也在依赖 structured buffer，只是 CPU 做 culling/batching。

二、架构定调（本阶段需要明确的共识）

- GPUScene 持久存在，作为 RenderWorld/RenderEngine 的渲染数据同步子系统。
- RenderPath 只负责渲染执行（Execute），不再负责 Render 数据更新。
- Scene 只负责逻辑更新与“脏标记”，RenderEngine 负责“消费脏标记并同步渲染数据”。
- Legacy 定义为“CPU Culling + GPU 数据结构复用”的路径，不再等价于“无 GPUScene”。

三、命名与数据归属整改（必须统一）

系统层级（建议命名）
- RenderWorld：CPU 侧渲染数据整理与快照构建（消费 Scene 的 dirty）。
- GpuScene：GPU 侧持久化数据与 Upload/Copy，同步 RenderWorld 产生的数据。
- Scene：逻辑与脏标记，不负责 GPU 同步。

数据归属（关键）
- Scene 只保留逻辑态与 dirty 标记。
- RenderSceneData 逻辑上归 RenderWorld（可暂时挂在 Scene，但只能被 RenderWorld 消费）。
- GpuScene 只持有 GPU Buffer 与资源索引（object/material/visibility/indirect）。

命名建议（最小可落地）
- GPUScene -> GpuScene（或 GpuSceneManager，但职责严格限定为 GPU 同步）
- renderSceneData -> RenderSceneData
- allObjectDataBuffer -> objectDataBuffer
- allMaterialDataBuffer -> materialDataBuffer
- visibilityBuffer -> visibleInstanceIndicesBuffer
- perFrameBatchBuffer -> perFrameUploadBuffer
- perFramelinearMemoryAllocator -> perFrameCpuAllocator
- drawIndirectParamMap -> indirectDrawParamMap
- LagacyRenderPath -> LegacyRenderPath（拼写修正）

单例建议（便于落地）
- SceneManager、RenderEngine 继续保持单例（当前已有）。
- RenderWorld、GpuScene 作为 RenderEngine 的子系统（生命周期跟随 RenderEngine），优先通过 RenderEngine 访问，避免全局单例扩散。

四、目标时序（清晰版）

帧级流程（不引入多帧 in‑flight，仅清晰职责）：

1) ResourceManager::Update()  // 先 finalize 资源
2) SceneManager::UpdateLogic()  // 脚本与组件逻辑
3) SceneManager::UpdateTransform()
4) SceneManager::CollectRenderDirty()
5) RenderEngine::PrepareRenderData()  // 同步 RenderSceneData -> GPUScene
6) RenderEngine::Submit()             // RenderPath.Execute
7) SceneManager::EndFrame()           // 清理 dirty list

五、职责划分（最关键）

Scene / Component
- 负责逻辑与变更标记（Transform / Material / Renderer 等 dirty）。
- 不直接调用 GPUSceneManager，不做 render 数据同步。

RenderSceneData（暂时可继续挂在 Scene 上，但逻辑上视为 RenderWorld 的数据）
- 存放渲染用的静态/动态数据与 dirty 列表。
- 只由 RenderEngine::PrepareRenderData() 消费。

GPUScene
- 负责将 RenderSceneData 的 dirty 集合写入 GPU Buffer。
- 不依赖具体 RenderPath，可被 Legacy 与 GPUScene 路径复用。

RenderPath
- 只做渲染执行（Execute），不再触碰 “更新/同步”。

六、最小改动方案（Phase 0）

目标：不改变功能，只清理时序。

1) Game::Update() 调整顺序
- 先 ResourceManager::Update()
- 再 SceneManager::Update()

2) Scene::Update() 拆分
- UpdateLogic()
- UpdateTransform()
- CollectRenderDirty()
- UpdateRenderSceneData() 如果保留，应该只做 CPU 数据准备，不调用 GPUScene

3) RenderEngine 新增 PrepareRenderData()
- 调用 GPUScene::Tick() 或等价逻辑（保持功能不变）
- 消费 Scene::renderSceneData 的 dirty list

4) RenderPath 里移除 GPUScene::Tick()
- RenderPath.Execute 只处理 Render 指令生成

七、接口草案（示意）

Game::Update():
- ResourceManager::Update()
- SceneManager::Update()  // 逻辑+transform+collect dirty

Game::Render():
- RenderEngine::PrepareRenderData()
- RenderEngine::Tick() or Submit()  // RenderPath.Execute

八、后续扩展（不在本阶段）

- 3‑Frame in‑flight 与 frame fence 管理
- Copy/Graphics/Compute queue 分离
- RenderWorld 从 Scene 中完全剥离

九、接续任务（Fence + Upload 重构，下一阶段）

背景（当前阻塞点）
- `Runtime/Core/Game.cpp` 与 `Runtime/Renderer/RenderEngine.cpp` 当前是“强等待”链路：主线程每帧等待渲染线程/GPU完成，CPU-GPU 基本锁步。
- `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp` 的 `CreateTextureBuffer/CreateBuffer/UploadBuffer` 仍包含 `WaitForRenderFinish(mImediatelyFence)`，资源上传是阻塞式。
- 这两处会互相叠加，导致上传与渲染并行度上不去。

阶段目标
- 先把“帧同步”从强等待改为基于 fence value 的可控等待（按 frame slot 或资源粒度）。
- 再把“资源上传”改为提交后异步完成（pending -> ready），不在上传函数内部等待。

执行顺序（必须按序）
1) Fence 基础设施统一
- 在 D3D12 层提供：`SignalFrameFence()`（返回 fence value）与 `WaitForFenceValue(value)`。
- 保留现有接口兼容，逐步替换 `WaitForRenderFinish/WaitForGpuFinished` 的直接调用点。

2) 主循环去强等待（先改 RenderEngine/Game）
- 改造 `RenderEngine::Update/Tick/EndFrame`，移除“每帧全局强等”。
- 引入 frame slot（建议 2~3）记录“本槽最近一次提交 fence value”，仅在复用该槽资源前等待。
- 保持功能正确优先，第一步不追求多队列，仅先消除锁步。

3) UploadManager 抽象（统一上传入口）
- 把 `CreateTextureBuffer/CreateBuffer/UploadBuffer` 里的立即提交+等待改为“提交上传请求”。
- 请求返回 `UploadTicket`（或等价句柄），支持 `callback` 或 `poll` 查询完成。
- 统一维护 pending upload 资源生命周期，按 fence 完成后回收 upload heap 临时资源。

4) 资源可见性切换（pending -> live）
- `Mesh/Texture` 上传后不立刻生效；完成后再把 allocation/texture handle 从 pending 切到 live。
- 渲染侧只读 live，未 ready 时使用默认资源或跳过绘制，避免读半成品。

5) 清理与回归
- 清理同步上传遗留路径，确保 Runtime 里没有“上传函数内部强等”。
- 增加 profiling 标记：主线程等待时长、上传完成延迟、帧内 copy 批次大小。

里程碑验收（下一阶段）
- M1：`Game/RenderEngine` 不再每帧全局阻塞等待 GPU 完成。
- M2：`D3D12RenderAPI` 上传路径不再调用 `WaitForRenderFinish(mImediatelyFence)`。
- M3：`Mesh/Texture` 具备 pending/ready 状态，渲染路径可处理未 ready 资源。

十、验收标准（本阶段）

- RenderPath 不再包含任何 “Render 数据更新” 操作。
- GPUScene 的 Tick 只在 RenderEngine 的 PrepareRenderData 阶段调用。
- Scene 更新路径里没有 GPU/RenderAPI 调用。
- Update 时序在 Game::Update / Game::Render 中清晰可读。
