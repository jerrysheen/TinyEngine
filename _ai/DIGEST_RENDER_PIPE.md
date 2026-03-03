# Architecture Digest: RENDER_PIPE
> Auto-generated. Focus: Runtime/Renderer, Runtime/Renderer/RenderPipeLine, Runtime/Renderer/RenderPath, RenderPipeLine, RenderPass, RenderPath, RenderContext, Batch, Culling, RenderEngine, Sort, Blit

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作，并建立解耦的帧更新流（GameObject/Component、Scene、CPUScene/GPUScene、FrameContext多帧同步）。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。
- 针对更新链路重点追踪：Game::Update/Render/EndFrame -> SceneManager/Scene -> CPUScene -> GPUScene -> FrameContext。
- 重点识别NodeDirtyFlags、NodeDirtyPayload、PerFrameDirtyList、CopyOp等脏数据传播与跨帧同步结构。

## Understanding Notes
- 渲染管线负责Pass组织、排序、裁剪与FrameGraph思路。
- 强调RenderContext、RenderPass与RenderPath的接口。

## Key Files Index
- `[70]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[68]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp** *(Content Included)*
- `[67]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[64]` **Runtime/Renderer/RenderPipeLine/RenderPass.h** *(Content Included)*
- `[62]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[62]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[61]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[59]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp** *(Content Included)*
- `[58]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp** *(Content Included)*
- `[54]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[52]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderPath/LagacyRenderPath.cpp** *(Content Included)*
- `[49]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h** *(Content Included)*
- `[48]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[47]` **Runtime/Renderer/RenderPath/IRenderPath.h** *(Content Included)*
- `[45]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[42]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[41]` **Runtime/Renderer/BatchManager.cpp** *(Content Included)*
- `[39]` **Runtime/Renderer/RenderSorter.h** *(Content Included)*
- `[38]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[36]` **Runtime/Renderer/Culling.cpp**
- `[35]` **Runtime/Renderer/Culling.h**
- `[30]` **Assets/Shader/GPUCulling.hlsl**
- `[25]` **Assets/Shader/BlitShader.hlsl**
- `[24]` **Runtime/Renderer/Renderer.h**
- `[22]` **Runtime/Renderer/Renderer.cpp**
- `[20]` **Runtime/Entry.cpp**
- `[19]` **Runtime/Core/Game.cpp**
- `[18]` **Runtime/Renderer/RenderAPI.h**
- `[17]` **Runtime/Renderer/RenderStruct.h**
- `[14]` **Runtime/Core/PublicStruct.h**
- `[14]` **Runtime/Renderer/FrameContext.h**
- `[13]` **Runtime/Renderer/FrameContext.cpp**
- `[12]` **Runtime/GameObject/Camera.cpp**
- `[12]` **Runtime/Renderer/PerDrawAllocator.h**
- `[12]` **Runtime/Renderer/RenderCommand.h**
- `[12]` **Runtime/Renderer/RenderUniforms.h**
- `[12]` **Runtime/Renderer/RenderWorld.h**
- `[12]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[12]` **Runtime/Scene/SceneManager.cpp**
- `[10]` **Runtime/Renderer/RenderAPI.cpp**
- `[6]` **Runtime/Scene/GPUScene.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **Runtime/Graphics/Material.cpp**
- `[5]` **Runtime/Scene/CPUScene.cpp**
- `[5]` **Runtime/Scene/GPUScene.cpp**
- `[5]` **Runtime/Settings/ProjectSettings.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[4]` **Runtime/GameObject/Camera.h**
- `[4]` **Runtime/Scene/SceneManager.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**
- `[4]` **Assets/Shader/StandardPBR.hlsl**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[3]` **Runtime/EngineCore.h**
- `[3]` **Runtime/Core/PublicEnum.h**
- `[3]` **Runtime/Graphics/GPUBufferAllocator.cpp**
- `[3]` **Runtime/Settings/ProjectSettings.cpp**

## Evidence & Implementation Details

### File: `Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    GPUSceneRenderPass::GPUSceneRenderPass()
    {
        Create();
    }

    void EngineCore::GPUSceneRenderPass::Create()
    {

    }
    
    void EngineCore::GPUSceneRenderPass::Configure(const RenderContext& context)
    {
        mRenderPassInfo.passName = "GPUSceneRenderPass";
        mRenderPassInfo.enableBatch = true;
        mRenderPassInfo.enableIndirectDrawCall = true;
        RenderTexture* colorAttachment = context.camera->colorAttachment;
        RenderTexture* depthAttachment = context.camera->depthAttachment;
        SetRenderTarget(colorAttachment, depthAttachment);
        SetViewPort(Vector2(0,0), Vector2(colorAttachment->GetWidth(), colorAttachment->GetHeight()));
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
    }
    
    // maybe send a context here?
    void EngineCore::GPUSceneRenderPass::Execute(RenderContext& context)
    {
        //// 每Pass设置一次
        //m_LastMatState.Reset();

        //Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        //Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        //Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        //Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        //if(!RenderSettings::s_EnableVertexPulling)
        //{
        //    for(auto& [hashID, renderContext] : BatchManager::GetInstance()->drawIndirectContextMap)
        //    {
        //        int batchID = BatchManager::GetInstance()->drawIndirectParamMap[hashID].indexInDrawIndirectList;
        //        int stratIndex = BatchManager::GetInstance()->drawIndirectParamMap[hashID].startIndexInInstanceDataList;
        //        Material* mat = renderContext.material;
        //        Mesh* mesh = renderContext.mesh;
        //        // 根据mat + pass信息组织pippeline
        //        Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        //        // copy gpu material data desc 
        //        Renderer::GetInstance()->SetBindlessMat(mat);
        //        // bind mesh vertexbuffer and indexbuffer.
        //        Renderer::GetInstance()->SetMeshData(mesh);
        //        Payload_DrawIndirect indirectPayload;
        //        // temp:
        //        GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
        //        ASSERT(indirectDrawArgsBuffer != nullptr);
        //        indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
        //        indirectPayload.count = 1;
        //        indirectPayload.startIndex = batchID;
        //        indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
        //        Renderer::GetInstance()->DrawIndirect(indirectPayload);
        //    }
        //}
        //else
        //{
        //    for(auto& [hashID, renderContext] : BatchManager::GetInstance()->drawIndirectContextMap)
        //    {
        //        int batchID = BatchManager::GetInstance()->drawIndirectParamMap[hashID].indexInDrawIndirectList;
        //        int stratIndex = BatchManager::GetInstance()->drawIndirectParamMap[hashID].startIndexInInstanceDataList;
        //        Material* mat = renderContext.material;
        //        if (mat->GetMaterialRenderState().GetHash() != m_LastMatState.GetHash())
        //        {
        //            m_LastMatState = mat->GetMaterialRenderState();
        //            Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        //            Renderer::GetInstance()->SetBindlessMat(mat);
        //            Renderer::GetInstance()->SetBindLessMeshIB(0);
        //        }

        //        Payload_DrawIndirect indirectPayload;
        //        // temp:
        //        GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
```

### File: `Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp`
```cpp

        // todo： 后面挪到别的地方， 先做Batch的部分：
        Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        
        Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        
        // 根据mat + pass信息组织pippeline
        Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        // copy gpu material data desc 
        Renderer::GetInstance()->SetMaterialData(mat);
        //Texture* tex = context.camera->colorAttachment.Get();
        //Renderer::GetInstance()->SetResourceState(tex, BufferResourceState::STATE_SHADER_RESOURCE);
        //Renderer::GetInstance()->BindTexture(tex, 0, 0);
        // bind mesh vertexbuffer and indexbuffer.
        Renderer::GetInstance()->SetMeshData(model);
        Renderer::GetInstance()->DrawIndexedInstanced(model, 1, PerDrawHandle{0,0});
```

### File: `Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp`
```cpp
    {

        PROFILER_EVENT_BEGIN("MainThread::OpaqueRenderPass::SetDrawCall");
        IssueRenderCommandCommon(mRenderPassInfo, mRenderPassInfo.renderBatchList);

        PROFILER_EVENT_END("MainThread::OpaqueRenderPass::SetDrawCall");

    }

    void OpaqueRenderPass::Filter(const RenderContext &context)
    {
```

### File: `Runtime/Renderer/RenderPipeLine/RenderPass.h`
```cpp
namespace EngineCore
{
    class RenderPass
    {
    public:

        // init RenderPass, for example Create FBO..
        //virtual void Create(const RenderContext& context) = 0;
        virtual void Create() = 0;
        
        // Record Rendertarget in mRenderPassInfo..
        virtual void Configure(const RenderContext& context) = 0;
        
        // decide what content we need to draw, for example
        // in opaquepass we only need gameobject renderqueue less than 3000.
        virtual void Filter(const RenderContext& context) = 0;
        
        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // CPU Prepare Data.
        virtual void Prepare(RenderContext& context) = 0;

        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // Submit To RenderThread
        virtual void Execute(RenderContext& context) = 0;
        
        // sent current Data to Renderer-> RenderPassInfo
        virtual void Submit() = 0;

        inline void Clear() { mRenderPassInfo.Reset(); };


        // ignore null ptr since RenderAPI level will handle this problem
        inline void SetRenderTarget(RenderTexture* colorAttachment, RenderTexture* depthAttachment)
        {
            mRenderPassInfo.colorAttachment = colorAttachment;
            mRenderPassInfo.depthAttachment = depthAttachment;
        };

        inline void SetClearFlag(ClearFlag flag, const Vector3& colorValue = Vector3::Zero, float depthValue = 1.0)
        {
            mRenderPassInfo.clearFlag = flag;
            mRenderPassInfo.clearColorValue = colorValue;
            mRenderPassInfo.clearDepthValue = depthValue;
        };

        inline void SetViewPort(const Vector2& startPos, const Vector2& endPos)
        {
            mRenderPassInfo.viewportStartPos = startPos;
            mRenderPassInfo.viewportEndPos = endPos;
        }

        inline const RenderPassInfo& GetRenderPassInfo(){return mRenderPassInfo;};
        RootSigSlot mRootSigSlot;
    protected:
        void IssueRenderCommandCommon(const RenderPassInfo& passInfo,
                                      const std::vector<RenderBatch>& batches
                                      );
    public:
        string name;
        RenderPassInfo mRenderPassInfo;
    };
} // namespace EngineCore
```

### File: `Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h`
```cpp
namespace EngineCore
{
    class GPUSceneRenderPass : public RenderPass
    {
    public:

        GPUSceneRenderPass();
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        virtual void Prepare(RenderContext& context) override{};
        // 执行具体的draw
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;
    private:
        MaterailRenderState m_LastMatState;
    };
```

### File: `Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h`
```cpp
namespace EngineCore
{
    class OpaqueRenderPass : public RenderPass
    {
    public:

        OpaqueRenderPass();
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        
        virtual void Prepare(RenderContext& context) override;


        // 执行具体的draw
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;
    };
```

### File: `Runtime/Renderer/RenderPipeLine/FinalBlitPass.h`
```cpp
namespace EngineCore
{
    class FinalBlitPass : public RenderPass
    {
        public:
        // renderpass 初始化
        //virtual void Create(const RenderContext& context) override;
        // todo Create 也需要参数
        virtual void Create() override;
        // 配置rt等
        virtual void Configure(const RenderContext& context) override;
        // 执行具体的draw
        virtual void Prepare(RenderContext& context) override;
        virtual void Execute(RenderContext& context) override;

        virtual void Filter(const RenderContext& context) override;

        virtual void Submit() override;

    private:
        Material* mat;
        Mesh* model;
    };
```

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPath.h`
```cpp
namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Execute(RenderContext& context) override;
        virtual void Prepare(RenderContext& context) override {};


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
```

### File: `Runtime/Renderer/RenderPath/LagacyRenderPath.h`
```cpp
namespace EngineCore
{
    class LagacyRenderPath : public IRenderPath
    {
    public:
        virtual ~LagacyRenderPath() override {};
        virtual void Execute(RenderContext& context) override;
        virtual void Prepare(RenderContext& context) override;
    };
```

### File: `Runtime/Renderer/RenderPath/IRenderPath.h`
```cpp
{
    class Renderer;
    class IRenderPath
    {
    public:
        virtual ~IRenderPath() = default;
        virtual void Execute(RenderContext& context) = 0;
        virtual void Prepare(RenderContext& context) = 0;
    };
```

### File: `Runtime/Renderer/RenderEngine.h`
```cpp
    // render: 执行场景渲染，不同管线进行渲染等。
    // end render : RenderAPI->EndFrame() 调用Present，buckbufferIndex Update
    class RenderEngine
    {
    public:
        static RenderEngine* GetInstance(){return s_Instance.get();};
        static bool IsInitialized(){return s_Instance != nullptr;};
        void Update(uint32_t frameID);
        static void Create();
        void Tick();
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
        ~RenderEngine(){};
        static void WaitForLastFrameFinished();
        static void SignalMainThreadSubmited();
        inline CPUScene& GetCPUScene(){return mCPUScene;}
        inline GPUScene& GetGPUScene(){return mGPUScene;}
        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCPUScene.SetCurrentFrame(currentFrame);
            mGPUScene.SetCurrentFrame(currentFrame);
        }
    private:
        IRenderPath* mCurrentRenderPath;
        static std::unique_ptr<RenderEngine> s_Instance;
        RenderContext renderContext;

        GPUScene mGPUScene;
        CPUScene mCPUScene;

        void ComsumeDirtySceneRenderNode();
        void UploadCopyOp();
    };
```

### File: `Runtime/Renderer/RenderContext.h`
```cpp
{
    // 记录灯光，阴影，物体信息？
    class RenderContext
    {
    public:
        RenderContext()
        {
            visibleItems.resize(10000);
        }
        Camera* camera;
        vector<LightData*> visibleLights;
        vector<RenderPacket> visibleItems;
        int currentMaxIndex = 0;

        inline void Reset()
        {
            camera = nullptr;
            visibleLights.clear();
            visibleItems.clear();
        }

        static void DrawRenderers(RenderContext& renderContext, 
                           const ContextDrawSettings& drawingSettings, 
                           const ContextFilterSettings& filteringSettings, 
                           std::vector<RenderBatch>& outDrawRecords);
        
        static void BatchContext(std::vector<RenderPacket*>& sortedItem);

        static bool CanBatch(const RenderBatch& batch, const RenderPacket item);

    private:

    };
```

### File: `Runtime/Renderer/RenderSorter.h`
```cpp
    }    

    class RendererSort
    {
    public:
        static void BuildSortKeys(const RenderContext& context, std::vector<RenderPacket>& items,
                                SortingCriteria sortingCriteria)
        {
            Vector3 camPos = context.camera->gameObject->transform->GetWorldPosition();
            float nearPlane = context.camera->mNear;
            float farPlane = context.camera->mFar;
            for(auto& item : items)
            {
                uint32_t meshID = item.meshID;
                uint32_t matID = item.materialID;

                float distance = item.distanToCamera;
                uint16_t distanceToID = FloatToDepth(distance, nearPlane, farPlane);
                switch(sortingCriteria)
                {
                    case SortingCriteria::ComonOpaque:
                    break;
                    case SortingCriteria::ComonTransparent:
                        distanceToID = 0xFFFF - distanceToID;
                    break;
                }
                // todo : 用游戏isntanceID，改成24位足够了
                uint64_t key = 0;
                key |= (uint64_t)(0) << 60; // Layer 0
                key |= (uint64_t)(matID & 0x0FFFFFFF) << 32; 
                key |= (uint64_t)(meshID & 0xFFFF) << 16;
                key |= (uint64_t)(distanceToID & 0xFFFF); 
                item.sortingKey = key;
            }
        }
    };
```

### File: `Runtime/Renderer/BatchManager.h`
```cpp

    class MeshRenderer;
    class BatchManager
    {
    public:
        static BatchManager* GetInstance()
        {
            if(s_Instance == nullptr)
            {
                Create();
            }
            return s_Instance;
        }
        void TryAddBatches(AssetID meshID, AssetID materialID, uint32_t layer); 
        void TryDecreaseBatches(AssetID meshID, AssetID materialID, uint32_t layer);
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;
        static std::unordered_map<uint64_t, int> BatchMap;
        
        std::vector<RenderProxy> GetAvaliableRenderProxyList(AssetID meshID, AssetID materialID, uint32_t layer); 
        
        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        uint64_t GetBatchHash(AssetID meshID, AssetID materialID, uint32_t layer); 
        
        static void Create();
        static BatchManager* s_Instance;

    };
```