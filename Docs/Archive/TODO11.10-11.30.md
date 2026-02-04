# TinyEngine 开发计划 (11月10日 - 11月30日)

## 📅 开发周期
**起始日期**: 2025年11月10日  
**结束日期**: 2025年11月30日  
**总时长**: 3周

---

## 🎯 总体目标

本阶段聚焦于**渲染管线升级**，核心目标包括：

1. ✅ **场景快速构建工具** - SceneBuilder（已完成）
2. 🔲 **视锥剔除系统** - AABB + Frustum Culling
3. 🔲 **多线程Job系统** - 并行化剔除
4. 🔲 **渲染批处理优化** - SRP Batch + GPU Instancing
5. 🔲 **材质排序系统** - 减少状态切换
6. 🔲 **新渲染Pass** - ShadowPass、SkyboxPass、TransparentPass

---

## 📋 详细任务清单

### **阶段0：场景构建工具（已完成）**
**日期**: 11月10日  
**状态**: ✅ 完成

- [x] 创建 `SceneBuilder` 类（Editor命名空间）
- [x] 实现 `CreateSampleScene()` - 示例场景创建
- [x] 实现 `CreateTestScene()` - 性能测试场景（可配置对象数量）
- [x] 实现 `CreateEmptyScene()` - 空场景创建
- [x] 集成到 EditorMainBar 菜单（Assets -> Create Scene）
- [x] 辅助函数：`CreateCameraObject()` 和 `CreateModelObject()`

**使用方式**：
```cpp
// 在Editor菜单中：Assets -> Create Scene
// - Sample Scene: 创建维京房屋示例场景
// - Empty Scene: 只包含相机的空场景
// - Test Scene (100 Objects): 用于性能测试的网格场景
```

---

### **Week 1：视锥剔除系统（11月11日 - 11月17日）**

#### **Day 1-2：AABB + Frustum 数学基础**
**日期**: 11月11日 - 11月12日  
**目标**: 实现包围盒和视锥体的数学结构

**任务清单**：
- [ ] 实现 `AABB` 结构体
  - [ ] 数据成员：`center` 和 `extents`（或 `min/max`）
  - [ ] 方法：`GetMin()`, `GetMax()`, `Transform(Matrix4x4)`
  - [ ] 工具方法：`Encapsulate(Vector3)`, `Intersects(AABB)`
  - [ ] 调试绘制：`DebugDraw()` - 绘制线框包围盒
- [ ] 实现 `Plane` 结构体
  - [ ] 数据成员：`normal` 和 `distance`
  - [ ] 方法：`GetDistanceToPoint(Vector3)`
- [ ] 实现 `Frustum` 结构体
  - [ ] 数据成员：6个平面（Left/Right/Top/Bottom/Near/Far）
  - [ ] 方法：`ExtractFromMatrix(Matrix4x4)` - 从VP矩阵提取视锥
  - [ ] 方法：`IntersectsAABB(AABB)` - AABB与视锥相交测试
  - [ ] 方法：`ContainsPoint(Vector3)` - 点是否在视锥内

**文件位置**：
- `Runtime/Math/AABB.h` 和 `AABB.cpp`
- `Runtime/Math/Frustum.h` 和 `Frustum.cpp`

**参考算法**：
```cpp
// Frustum提取（Gribb-Hartmann方法）
// VP矩阵行提取6个平面
// Left:   VP[3] + VP[0]
// Right:  VP[3] - VP[0]
// Bottom: VP[3] + VP[1]
// Top:    VP[3] - VP[1]
// Near:   VP[3] + VP[2]
// Far:    VP[3] - VP[2]
```

#### **Day 3-4：集成到MeshRenderer和Camera**
**日期**: 11月13日 - 11月14日  
**目标**: 为渲染对象添加AABB，相机计算Frustum

**任务清单**：
- [ ] 扩展 `ModelData` 类
  - [ ] 加载Mesh时计算本地空间AABB
  - [ ] 存储AABB到模型数据中
- [ ] 扩展 `MeshRenderer` 组件
  - [ ] 添加成员：`AABB worldAABB`
  - [ ] 添加方法：`UpdateWorldAABB()` - 根据Transform更新
  - [ ] 添加方法：`GetWorldAABB() const`
- [ ] 扩展 `Camera` 类
  - [ ] 添加方法：`GetFrustum()` - 提取当前视锥
  - [ ] 缓存Frustum避免每帧重复计算
- [ ] 集成到Transform更新流程
  - [ ] Transform变化时标记需要更新AABB
  - [ ] Scene Update时批量更新所有脏AABB

**设计要点**：
- AABB更新应该是lazy的（只在需要时计算）
- 使用脏标记避免重复计算
- 世界空间AABB = 本地AABB经过Transform矩阵变换

#### **Day 5-7：Culling系统改造**
**日期**: 11月15日 - 11月17日  
**目标**: 实现基于Frustum的视锥剔除

**任务清单**：
- [ ] 改造 `Culling::Run()` 函数
  - [ ] 从Camera提取Frustum
  - [ ] 遍历场景对象进行AABB-Frustum测试
  - [ ] 只将可见对象添加到RenderContext
- [ ] 添加剔除统计
  - [ ] 结构体：`CullingStats`（总对象数、可见数、剔除数、耗时）
  - [ ] 静态成员存储统计信息
  - [ ] 提供查询接口：`GetStats()`
- [ ] 调试可视化
  - [ ] 绘制所有AABB（绿色=可见，红色=剔除）
  - [ ] 绘制Camera的Frustum
  - [ ] ImGui面板显示剔除统计
- [ ] 性能测试
  - [ ] 使用SceneBuilder创建TestScene(1000)
  - [ ] 对比启用/禁用剔除的性能差异
  - [ ] 记录剔除率和FPS提升

**预期效果**：
- 100个物体场景：剔除率约50%（取决于相机角度）
- 1000个物体场景：剔除率约80-90%（大部分在视野外）
- DrawCall数量明显减少

---

### **Week 2：多线程Job系统 + 渲染批处理（11月18日 - 11月24日）**

#### **Day 8-10：简单Job系统实现**
**日期**: 11月18日 - 11月20日  
**目标**: 实现线程池和任务调度

**任务清单**：
- [ ] 实现 `JobSystem` 类
  - [ ] 线程池初始化（默认使用硬件并发数）
  - [ ] Job提交接口：`Schedule(JobFunc)`
  - [ ] 并行循环接口：`ParallelFor(start, end, func, batchSize)`
  - [ ] 关闭和清理接口
- [ ] Job工作窃取队列（可选）
  - [ ] 每个线程独立队列
  - [ ] 空闲线程可以窃取其他线程任务
- [ ] 测试Job系统
  - [ ] 简单的并行计算测试
  - [ ] 验证线程安全性

**文件位置**：
- `Runtime/Core/JobSystem.h` 和 `JobSystem.cpp`

**设计参考**：
```cpp
// 使用std::thread + std::function + std::condition_variable
// 或者使用C++17的std::async + std::future
```

#### **Day 11-12：多线程剔除**
**日期**: 11月21日 - 11月22日  
**目标**: 将剔除任务并行化

**任务清单**：
- [ ] 改造 `Culling::Run()` 为多线程版本
  - [ ] 将场景对象分块（每块64-128个对象）
  - [ ] 每个线程处理一块，独立进行剔除测试
  - [ ] 使用thread-local结果缓存避免锁竞争
  - [ ] 主线程合并所有结果到RenderContext
- [ ] 性能对比测试
  - [ ] 单线程 vs 多线程剔除耗时
  - [ ] 不同对象数量下的加速比
  - [ ] CPU占用率监控
- [ ] 优化内存分配
  - [ ] 使用对象池避免频繁new/delete
  - [ ] RenderPacket预分配

**预期效果**：
- 1000+对象场景：剔除耗时减少50-70%
- 4核CPU约3倍加速比

#### **Day 13-14：材质排序和RenderBatch**
**日期**: 11月23日 - 11月24日  
**目标**: 实现渲染批次构建和排序

**任务清单**：
- [ ] 实现 `RenderBatch` 结构体
  - [ ] 数据成员：`shader`, `material`, `mesh`, `worldMatrices[]`
  - [ ] 方法：`GetSortKey()` - 生成64位排序键
  - [ ] 运算符：`operator<` 用于排序
- [ ] 实现 `RenderBatchManager` 类
  - [ ] 静态方法：`BuildBatches(RenderPackets)` - 构建批次
  - [ ] 静态方法：`CanBatch(item1, item2)` - 判断是否可批处理
- [ ] 批次构建算法
  - [ ] 按Shader > Material > Mesh排序
  - [ ] 合并连续相同材质和Mesh的物体
  - [ ] 每个批次记录所有实例的世界矩阵
- [ ] 集成到渲染管线
  - [ ] OpaqueRenderPass使用批次渲染
  - [ ] 统计批次数量和DrawCall减少率

**排序键设计**：
```
[63:32] Shader指针哈希（高32位）
[31:16] Material指针哈希（中16位）
[15:0]  Mesh指针哈希（低16位）
```

---

### **Week 3：GPU Instancing + 新Pass系统（11月25日 - 11月30日）**

#### **Day 15-17：GPU Instancing实现**
**日期**: 11月25日 - 11月27日  
**目标**: 支持GPU实例化渲染

**任务清单**：
- [ ] 创建Instance Buffer
  - [ ] D3D12实现：StructuredBuffer（存储矩阵数组）
  - [ ] 每帧动态更新Instance Buffer
- [ ] Shader支持Instancing
  - [ ] 修改顶点Shader接收`SV_InstanceID`
  - [ ] 从StructuredBuffer读取实例矩阵
  - [ ] 测试Shader：`InstancedShader.hlsl`
- [ ] Renderer添加Instancing路径
  - [ ] 方法：`DrawIndexedInstanced(meshID, instanceCount)`
  - [ ] 方法：`UploadInstanceBuffer(matrices)`
  - [ ] 判断：instanceCount > 1时使用Instancing
- [ ] 批处理集成
  - [ ] RenderBatch自动检测可实例化批次
  - [ ] 相同Mesh+Material的多个物体合批
- [ ] 性能测试
  - [ ] 100个相同物体：DrawCall从100降到1
  - [ ] 对比传统渲染和Instancing的FPS

**预期效果**：
- 100个相同材质物体：1个DrawCall（vs 100个）
- GPU利用率提升，CPU瓶颈缓解

#### **Day 18-19：ShadowPass实现**
**日期**: 11月28日  
**目标**: 添加阴影渲染Pass

**任务清单**：
- [ ] 实现 `ShadowPass` 类
  - [ ] 继承自RenderPass
  - [ ] 创建Shadow Map纹理（2048x2048深度纹理）
  - [ ] Configure: 设置深度渲染目标
  - [ ] Execute: 从光源视角渲染场景
- [ ] 光源阴影相机
  - [ ] 方向光：使用正交投影
  - [ ] 计算Light View-Projection矩阵
- [ ] 主相机采样Shadow Map
  - [ ] 传递Shadow Map到OpaquePass
  - [ ] Shader实现阴影采样和PCF
- [ ] 调试可视化
  - [ ] 显示Shadow Map内容
  - [ ] 绘制光源视锥

**文件位置**：
- `Runtime/Renderer/RenderPipeLine/ShadowPass.h` 和 `.cpp`
- `Assets/Shader/ShadowCaster.hlsl` - 阴影投射Shader
- `Assets/Shader/ShadowReceiver.hlsl` - 阴影接收Shader

#### **Day 20：SkyboxPass + TransparentPass**
**日期**: 11月29日  
**目标**: 添加天空盒和半透明渲染Pass

**任务清单**：
- [ ] 实现 `SkyboxPass` 类
  - [ ] 使用Cubemap纹理
  - [ ] 渲染立方体（内翻面）
  - [ ] 深度测试设置为LEqual（绘制到远平面）
- [ ] 实现 `TransparentPass` 类
  - [ ] Filter: 筛选RenderQueue >= 3000的物体
  - [ ] 从后往前排序（基于到相机距离）
  - [ ] 关闭深度写入，开启Alpha混合
  - [ ] Execute: 渲染半透明物体

**文件位置**：
- `Runtime/Renderer/RenderPipeLine/SkyboxPass.h` 和 `.cpp`
- `Runtime/Renderer/RenderPipeLine/TransparentPass.h` 和 `.cpp`

#### **Day 21：集成与测试**
**日期**: 11月30日  
**目标**: 完整测试新渲染管线

**任务清单**：
- [ ] 配置RenderPassAsset
  - [ ] 顺序：ShadowPass → OpaquePass → SkyboxPass → TransparentPass → FinalBlit
- [ ] 完整场景测试
  - [ ] 创建包含各种物体的综合场景
  - [ ] 测试阴影、天空盒、半透明效果
- [ ] 性能Profiling
  - [ ] 各Pass耗时统计
  - [ ] DrawCall统计（启用/禁用Batch对比）
  - [ ] 剔除效率统计
  - [ ] GPU占用率监控
- [ ] ImGui调试面板
  - [ ] 显示渲染统计信息
  - [ ] 切换各种可视化模式
  - [ ] 实时调整剔除和批处理参数

---

## 📊 预期性能指标

| 指标 | 改造前 | 改造后（目标） | 提升幅度 |
|------|--------|----------------|----------|
| **DrawCall数** | N（物体数） | N/10（批处理后） | 减少90% |
| **剔除方式** | 无剔除 | Frustum + 多线程 | 新增功能 |
| **剔除耗时** | 0ms | <1ms (1000物体) | - |
| **可见物体** | 100% | 20-50%（取决于场景） | - |
| **渲染Pass** | 2个 | 5个（完整管线） | 新增3个 |
| **材质切换次数** | N | <N/5（排序后） | 减少80% |
| **GPU利用率** | 30-40% | 70-80% | 提升2倍 |
| **FPS（1000物体）** | 30-40 | 100-120 | 提升3倍 |

---

## 🔍 技术难点和解决方案

### **1. Frustum提取精度问题**
**问题**: 从VP矩阵提取的平面可能不准确  
**解决**: 
- 使用Gribb-Hartmann方法（行提取）
- 归一化平面法线
- 测试已知点验证正确性

### **2. AABB旋转更新**
**问题**: 非轴对齐变换后AABB不准确  
**解决**:
- 变换8个顶点后重新计算包围盒
- 或使用OBB（有向包围盒）
- 或保守扩大AABB

### **3. 多线程剔除的数据竞争**
**问题**: 多个线程同时写RenderContext  
**解决**:
- 每个线程独立结果缓存
- 主线程单独合并结果
- 使用对象池避免分配竞争

### **4. GPU Instancing的Shader兼容性**
**问题**: 旧Shader不支持Instancing  
**解决**:
- 提供Instanced和Non-Instanced两版本
- Material标记是否支持Instancing
- 自动降级到传统渲染

### **5. Shadow Map精度和走样**
**问题**: 阴影边缘锯齿、阴影痤疮  
**解决**:
- 使用PCF（Percentage Closer Filtering）
- Bias偏移避免自阴影
- Cascade Shadow Map（后续优化）

---

## 🎯 里程碑验收标准

### **Week 1 完成标志**
- ✅ 1000个物体场景可视锥剔除生效
- ✅ 剔除率达到80%以上（视角移出时）
- ✅ AABB可视化正常显示
- ✅ 剔除耗时 < 2ms

### **Week 2 完成标志**
- ✅ Job系统正常运行，无死锁/崩溃
- ✅ 多线程剔除比单线程快2倍以上
- ✅ DrawCall数量减少70%以上
- ✅ 批处理统计面板可显示

### **Week 3 完成标志**
- ✅ GPU Instancing渲染100个相同物体只需1个DrawCall
- ✅ 阴影正常显示，无严重走样
- ✅ 天空盒和半透明物体渲染正确
- ✅ 完整渲染管线FPS达到目标（1000物体 > 60fps）

---

## 💡 后续优化方向（P1）

1. **空间分区加速**
   - Octree / BVH 代替线性遍历
   - 适用于超大场景（10000+物体）

2. **遮挡剔除**
   - 软件光栅化或GPU Occlusion Query
   - 室内场景性能提升显著

3. **LOD系统**
   - 根据距离切换模型细节
   - 减少三角形数量

4. **Indirect Drawing**
   - GPU驱动的DrawCall生成
   - 完全消除CPU瓶颈

5. **延迟渲染**
   - GBuffer Pass + Lighting Pass
   - 支持大量动态光源

6. **Compute Shader剔除**
   - GPU端执行剔除
   - 完全并行化

---

## 📝 开发日志

### 2025-11-10
- ✅ 完成 SceneBuilder 类实现
- ✅ 集成到 EditorMainBar 菜单
- ✅ 测试三种场景创建功能正常

### 2025-11-11
- 🔲 开始 AABB 和 Frustum 实现
- ...

---

## 🔗 相关文档

- [TODO10-11月.md](./TODO10-11月.md) - 上一阶段开发计划
- [TODO10-11月2.md](./TODO10-11月2.md) - Component和ECS设计讨论
- [组件工厂架构说明.md](./组件工厂架构说明.md) - 序列化系统说明
- [功能实现.md](./功能实现.md) - 已完成功能清单

---

## ✅ 任务总结

**总计**: 50+ 个子任务  
**已完成**: 6 个  
**进行中**: 0 个  
**待开始**: 44+ 个

**预计完成率**: 
- Week 1: 30%
- Week 2: 60%
- Week 3: 100%

---

祝开发顺利！🚀

