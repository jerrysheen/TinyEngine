<!-- Created by TinyEngine harness, task analyze-all-commits, step checkpoint-088-8ceccba -->
# Checkpoint 088: 8ceccba

## Commit 信息
- **Hash**: `8ceccba5b711bb3858bc3ad2a8cb7016ef8f4441`
- **Subject**: D: 第一版本IndirectDrawCall的数据准备，归并GPUBufferAllocator，创建BatchManager，初始化时计算出当前的Batch
- **Date**: 2025-12-29 11:40:27 +0800
- **Author**: shentao

## 核心变更
1. **IndirectDrawCall 数据准备**: 第一版间接绘制数据准备工作
2. **GPUBufferAllocator 归并**: 整合 GPU 缓冲区分配器
3. **BatchManager 新增**: 新增 `BatchManager.h/cpp` (212行)，管理批次数据
4. **删除旧分配器**: 删除 `LinearAllocateBuffer.h/cpp` (81行) 和 `PersistantBuffer.h/cpp` (135行)
5. **GPUSceneManager 重构**: 192行重构，适配新的分配器架构
6. **MeshFilter 扩展**: `MeshFilter.h/cpp` (24行)，支持间接绘制数据

## 涉及的模块
- **Runtime/Renderer**: `BatchManager.h/cpp` (212行，新增核心), `RenderCommand.h`, `RenderStruct.h`, `RenderUniforms.h`, `GPUSceneRenderPath.h` (48行)
- **Runtime/Graphics**: `GPUBufferAllocator.h/cpp` (217行，新增), `GPUSceneManager.h/cpp` (223行重构), `IGPUBuffer.h`
- **Runtime/GameObject**: `MeshFilter.h/cpp` (24行), `MeshRenderer.h/cpp`
- **Runtime/Core**: `PublicStruct.h` (27行), `LinearAllocator.h`, `Game.cpp`, `Game.h`
- **Runtime/Scene**: `Scene.h/cpp`, `SceneManager.h/cpp`
- **Assets/Shader**: `GPUCulling.hlsl` (36行)
- **删除**: `LinearAllocateBuffer.h/cpp`, `PersistantBuffer.h/cpp`

## 风险点
1. **大规模模块替换**: 删除 216 行旧代码，新增 429 行新代码
2. **BatchManager 新实现**: 212 行全新代码，需要充分测试
3. **分配器架构变更**: GPUBufferAllocator 替代 PersistantBuffer
4. **IndirectDraw 未完成**: commit message 明确提到"剩下后半部分"

## 可能的错误原因
1. **Buffer 分配冲突**: 新的 GPUBufferAllocator 与旧代码的资源释放冲突
2. **Batch 计算错误**: 初始化时计算的 Batch 数据不准确
3. **删除代码残留引用**: PersistantBuffer 和 LinearAllocateBuffer 的残留引用
4. **MeshFilter 数据不一致**: 新的 MeshFilter 数据结构与 GPU 端不匹配
5. **Scene 数据同步**: 29 行 Scene.cpp 更新后的数据同步问题
