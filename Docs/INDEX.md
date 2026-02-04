# Docs Index (更新: 2026-02-04)

文档结构
`Docs/TODO/` 进行中的任务清单
`Docs/Design/` 设计/规格/路线图
`Docs/Archive/` 历史记录与已归档想法
`Docs/Design/*.excalidraw` 图示文件

进行中 TODO
- `Docs/TODO/Update_RenderSync.md`
- `Docs/TODO/渲染线程与主线程并行.md`
- `Docs/TODO/Material重构_2025-12.md`
- `Docs/TODO/BindlessResourceManager_2026-01.md`
- `Docs/TODO/已知技术债.md`

设计与规格
- `Docs/Design/功能实现.md`
- `Docs/Design/GPUScene_Roadmap_2025-12.md`
- `Docs/Design/GPUBuffer重构设计.mermaid`
- `Docs/Design/组件工厂架构说明.md`（候选方案，尚未落地）
- `Docs/Design/PerDrawData.md`
- `Docs/Design/RootSignature_Refactor_Completed.mc`（基本完成）
- `Docs/Design/渲染模式与多帧同步笔记.md`

历史/归档
- `Docs/Archive/Done.md`（历史对话记录，含旧计划）
- `Docs/Archive/Done10-11月.md`（历史计划）
- `Docs/Archive/Todo10-11月Component和ECS.md`（历史讨论）
- `Docs/Archive/TODO11.10-11.30.md`（历史阶段计划）
- `Docs/Archive/可插拔模式性能比较.md`（实验性方案，暂不排期，视为可废弃）

图示（Design/）
- `Docs/Design/FBOManager.excalidraw`
- `Docs/Design/Material设计.excalidraw`
- `Docs/Design/Mesh设计.excalidraw`
- `Docs/Design/渲染管线概述.excalidraw`
- `Docs/Design/渲染资源生命周期.excalidraw`
- `Docs/Design/無標題-2025-07-07-1203.excalidraw`

对照 _ai 的状态概览
| 领域 | 现状 | 依据 | 备注 |
| --- | --- | --- | --- |
| 渲染管线基础 | 已有 | `_ai/DIGEST_RENDER_PIPE.md` | RenderPath/RenderPass/RenderContext 已成型 |
| GPUScene 主链路 | 部分完成 | `_ai/DIGEST_GPUScene.md` | GPUCulling + Indirect 已通，持久化/全量 GPUScene 仍在路线图中 |
| RHI/D3D12 基础 | 已有 | `_ai/DIGEST_RHI_D3D12.md` | D3D12RenderAPI/RootSignature 已具备 |
| 材质/资源系统 | 已有（基础） | `_ai/DIGEST_MATERIAL_SYSTEM.md`, `_ai/DIGEST_GRAPHICS_RES.md` | Bindless/全局资源管理尚未完善 |
| 场景/组件/Transform | 已有（基础） | `_ai/DIGEST_SCENE_SYSTEM.md`, `_ai/DIGEST_GAMEPLAY.md` | 生命周期调度仍需梳理（见 Update_RenderSync） |
| 剔除与批处理 | 已有（CPU侧） | `_ai/DIGEST_RENDER_PIPE.md`, `_ai/DIGEST_GPUScene.md` | GPU 驱动裁剪/批次进一步优化待做 |
| 多帧 in-flight 与同步 | 未完成 | `_ai/DIGEST_Concurrency.md` | 当前为锁步事件模型，见并行化 TODO |
| Copy/Graphics 队列分离 | 未完成 | `_ai/DIGEST_GPU_ARCHITECTURE.md` | 仍为单队列模型 |
| Prefab/组件工厂 | 未完成 | `Docs/Design/组件工厂架构说明.md` | 作为候选方案保留 |

