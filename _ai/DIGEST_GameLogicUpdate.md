# Architecture Digest: GameLogicUpdate
> Auto-generated. Focus: Runtime/Core/Game.h, Runtime/Core/Game.cpp, Game, SceneManager, Scene, CPUScene, GPUScene, RenderEngine, LagacyRenderPath, GPUSceneRenderPath, Renderer, RenderContext, FrameContext, SetCurrentFrame, UpdateDirtyNode, ApplyDirtyNode, UploadCopyOp, FinalBlitPass, GPUSceneRenderPass, OpaqueRenderPass, RenderPipeLine, EndFrame

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
- 围绕主循环的Update/Render/EndFrame与场景刷新顺序。
- 关注渲染路径选择与关键Pass调用链，明确Resource/Scene/RenderEngine时序和首帧行为。

## Key Files Index
- `[110]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[96]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp** *(Content Included)*
- `[94]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[92]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.h** *(Content Included)*
- `[77]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[67]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[66]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[64]` **Runtime/Scene/GPUScene.cpp** *(Content Included)*
- `[64]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[64]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp** *(Content Included)*
- `[61]` **Runtime/Scene/GPUScene.h** *(Content Included)*
- `[60]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[56]` **Runtime/Scene/CPUScene.cpp** *(Content Included)*
- `[56]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[55]` **Runtime/Scene/CPUScene.h** *(Content Included)*
- `[52]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[49]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[47]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[47]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.h** *(Content Included)*
- `[45]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.cpp**
- `[40]` **Runtime/GameObject/MeshRenderer.h**
- `[39]` **Runtime/Scene/Scene.h**
- `[39]` **Runtime/Renderer/RenderPath/IRenderPipeline.h**
- `[38]` **Runtime/Renderer/RenderContext.h**
- `[38]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[38]` **Runtime/Scene/Scene.cpp**
- `[38]` **Runtime/Serialization/SceneLoader.h**
- `[37]` **Runtime/Core/Game.h**
- `[33]` **Runtime/GameObject/Camera.cpp**
- `[33]` **Runtime/GameObject/GameObject.h**
- `[33]` **Runtime/Scene/BistroSceneLoader.h**
- `[32]` **Runtime/GameObject/GameObject.cpp**
- `[31]` **Runtime/Scene/SceneStruct.h**
- `[30]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[29]` **Runtime/Renderer/Culling.cpp**
- `[28]` **Editor/Panel/EditorGameViewPanel.cpp**
- `[24]` **Runtime/Entry.cpp**
- `[24]` **Runtime/Renderer/RenderBackend.h**
- `[24]` **Editor/Panel/EditorGameViewPanel.h**
- `[22]` **Runtime/GameObject/MeshFilter.cpp**
- `[21]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp**
- `[20]` **Runtime/GameObject/Transform.cpp**
- `[19]` **Runtime/GameObject/Transform.h**
- `[19]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[18]` **Runtime/Graphics/Material.cpp**
- `[18]` **Runtime/Renderer/BatchManager.h**
- `[18]` **Runtime/Renderer/RenderCommand.h**
- `[18]` **Editor/Panel/EditorMainBar.cpp**
- `[16]` **Runtime/Renderer/RenderBackend.cpp**
- `[16]` **Runtime/Renderer/RenderSorter.h**
- `[15]` **Runtime/GameObject/Component.h**
- `[15]` **Runtime/Renderer/Culling.h**
- `[15]` **Runtime/Renderer/RenderAPI.h**
- `[14]` **Runtime/GameObject/MeshFilter.h**
- `[14]` **Runtime/GameObject/MonoBehaviour.h**
- `[14]` **Runtime/Renderer/UploadPagePool.h**
- `[13]` **Runtime/GameObject/Camera.h**
- `[13]` **Runtime/GameObject/ComponentType.h**
- `[13]` **Runtime/GameObject/MonoBehaviour.cpp**

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
        //        GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::GPUSceneRenderPipeline.indirectDrawArgsBuffer;
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
        //        GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::GPUSceneRenderPipeline.indirectDrawArgsBuffer;
```

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp`
```cpp
#include "Renderer/RenderEngine.h"

namespace EngineCore
{
    void GPUSceneRenderPipeline::RecordAndFlush(RenderContext& context)
    {
//        if (!hasSetUpBuffer) 
//        {
//            hasSetUpBuffer = true;
//
//            BufferDesc desc;
//
//            desc.debugName = L"CullingParamBuffer";
//            desc.memoryType = BufferMemoryType::Upload;
//            desc.size = sizeof(GPUCullingParam);
//            desc.stride = sizeof(GPUCullingParam);
//            desc.usage = BufferUsage::ConstantBuffer;
//            cullingParamBuffer = new GPUBufferAllocator(desc);
//            cullingParamAlloc = cullingParamBuffer->Allocate(sizeof(Frustum));
//            
//            
//            desc.debugName = L"IndirectDrawArgsBuffer";
//            desc.memoryType = BufferMemoryType::Default;
//            desc.size = sizeof(DrawIndirectArgs) * 3000;
//            desc.stride = sizeof(DrawIndirectArgs);
//            desc.usage = BufferUsage::StructuredBuffer;
//            indirectDrawArgsBuffer = new GPUBufferAllocator(desc);
//            indirectDrawArgsAlloc = indirectDrawArgsBuffer->Allocate(sizeof(DrawIndirectArgs) * 3000);
//        }
//
//        FrameContext* currentFrame = RenderEngine::GetInstance()->GetGPUScene().GetCurrentFrameContext();
//        auto* visibilityBuffer = currentFrame->visibilityBuffer;
//        RenderBackend::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
//        RenderBackend::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
//
//
//        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
//        int gameObjectCount = SceneManager::GetInstance()->GetCurrentScene()->allObjList.size();
//
//        GPUCullingParam cullingParam;
//        cullingParam.frustum = cam->mFrustum;
//        cullingParam.totalItem = gameObjectCount;
//        cullingParamBuffer->UploadBuffer(cullingParamAlloc, &cullingParam, sizeof(GPUCullingParam));
//
//        
//        // Get Current BatchInfo:
//        vector<DrawIndirectArgs> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
//        if (batchInfo.size() != 0) 
//        {
//            indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
//        }
//
//        ComputeShader* csShader = RenderEngine::GetInstance()->GetGPUScene().GetCullingShaderHandler().Get();
//        csShader->SetBuffer("CullingParams", cullingParamBuffer->GetGPUBuffer());
//        csShader->SetBuffer("g_InputAABBs", currentFrame->allAABBBuffer->GetGPUBuffer());
//        csShader->SetBuffer("g_InputPerObjectDatas", currentFrame->allObjectDataBuffer->GetGPUBuffer());
//        csShader->SetBuffer("g_RenderProxies", currentFrame->renderProxyBuffer->GetGPUBuffer());
//        csShader->SetBuffer("g_VisibleInstanceIndices", currentFrame->visibilityBuffer->GetGPUBuffer());
//        csShader->SetBuffer("g_IndirectDrawCallArgs", indirectDrawArgsBuffer->GetGPUBuffer());
//
//        
//        Payload_DispatchComputeShader payload;
//        payload.csShader = RenderEngine::GetInstance()->GetGPUScene().GetCullingShaderHandler().Get();
//        payload.groupX = gameObjectCount / 64 + 1;
//        payload.groupY = 1;
//        payload.groupZ = 1;
//        RenderBackend::GetInstance()->DispatchComputeShader(payload);
//        // 先把shader跑起来， 渲染到RT上， 后续blit啥的接入后面再说
//
//        // 这个地方简单跑个绑定测试？
//        RenderBackend::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_SHADER_RESOURCE);
//        RenderBackend::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_INDIRECT_ARGUMENT);
//
//        context.Reset();
//        context.camera = cam;
//        //RenderBackend::GetInstance()->Render(context);
//#ifdef EDITOR
//        RenderBackend::GetInstance()->OnDrawGUI();
//        EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
//        EngineEditor::EditorGUIManager::GetInstance()->Render();
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

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.h`
```cpp
namespace EngineCore
{
    class GPUSceneRenderPipeline : public IRenderPipeline
    {
    public:
        virtual ~GPUSceneRenderPipeline() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Prepare(RenderContext& context) override {};
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

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
        void PrepareFrame(uint32_t frameID, const SceneDelta& delta);
        static void Create();
        void BuildFrame();
        void WaitForFrameAvaliable(uint32_t frameID);
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
        ~RenderEngine(){};
        //static void WaitForLastFrameFinished();
        //static void SignalMainThreadSubmited();
        inline CPUScene& GetCPUScene(){return mCPUScene;}
        inline GPUScene& GetGPUScene(){return mGPUScene;}
        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCPUScene.SetCurrentFrame(currentFrame);
            mGPUScene.SetCurrentFrame(currentFrame);
        }

        inline FrameTicket* GetCurrentFrameTicket(uint32_t frameID)
        {
            return &mFrameTicket[frameID % 3];
        }

        inline uint32_t GetCurrentFrame(){ return mCurrentFrameID;}
    private:
        IRenderPipeline* mCurrentRenderPipeline;
        
        static std::unique_ptr<RenderEngine> s_Instance;
        RenderContext renderContext;
        UploadPagePool* mUploadPagePool;
        GPUScene mGPUScene;
        CPUScene mCPUScene;
        static constexpr int  MAX_FRAME_INFLIGHT = 3;
        
        FrameTicket* mCurrFrameTicket;
        FrameTicket mFrameTicket[MAX_FRAME_INFLIGHT];
        uint32_t mCurrentFrameID = 0;

        void ComsumeDirtySceneRenderNode(const SceneDelta& delta);
        void UploadCopyOp();
    };
```

### File: `Runtime/Scene/GPUScene.h`
```cpp
namespace EngineCore
{
    class CPUSceneView;
    class GPUScene
    {
    public:
        GPUScene();
        void Create();
        void Update(uint32_t currentFrameIndex);
        void Destroy();
        void EndFrame();
        void BeginFrame();
        
        BufferAllocation GetSinglePerMaterialData();
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void UpdatePerFrameDirtyNode(CPUSceneView& view);
        void UploadCopyOp();
        
        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);
        void UpdateDirtyFlags(uint32_t renderID, uint32_t flags);
        void UpdateShadowData(uint32_t renderID, CPUSceneView& cpuScene);

        inline uint32_t GetCurrentFrameID() const { return mCurrentFrameID; }
        inline uint32_t GetMaxFrameCount() const { return MAX_FRAME_INFLIGHT; }

        inline GPUBufferAllocator* GetAllMaterialDataBuffer() { return allMaterialDataBuffer; }
        inline ResourceHandle<ComputeShader> GetCullingShaderHandler() { return GPUCullingShaderHandler; }

        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCurrentFrameID = currentFrame;
        }

        inline void SetUploadPagePool(UploadPagePool* pool)
        {
            mUploadPagePool = pool;
        }

        inline uint64_t GetAllObjectDataBufferAddress() { return allObjectDataBuffer->GetBaseGPUAddress(); }
        inline uint64_t GetAllAABBDataBufferAddress() { return allAABBBuffer->GetBaseGPUAddress(); }
        inline uint64_t GetCurrentVisibilityBuffer(uint32_t frameID) { return GetVisibilityBufferByFrameID(frameID)->GetBaseGPUAddress(); }
        inline GPUBufferAllocator* GetVisibilityBufferByFrameID(uint32_t frameID) { return visibilityBuffer[frameID % 3]; }
    private:
        void EnsureCapacity(uint32_t renderID);
    private:
        void SetCurrentContext();
        void TryFreeRenderProxyByRenderIndex(uint32_t renderID);
        void TryFreePerObjectDataAndAABBData(uint32_t renderID);
        
        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* renderProxyBuffer;
        GPUBufferAllocator* allMaterialDataBuffer;

        
        vector<uint32_t> mDirtyFlags;
        vector<uint32_t> mPerFrameDirtyID;
        vector<PerObjectData> mPerObjectDatas;

        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
        static const int MAX_FRAME_INFLIGHT = 3;
        uint32_t mCurrentFrameID = 0;

        UploadPagePool* mUploadPagePool;


        std::vector<CopyOp>* mCurrentCopyOp;
        std::vector<CopyOp> mFrameCopyOp[MAX_FRAME_INFLIGHT];
        
        GPUBufferAllocator* mCurrVisibilityBuffer;
        GPUBufferAllocator* visibilityBuffer[3];
    };

}
```

### File: `Runtime/Scene/SceneManager.h`
```cpp
namespace EngineCore
{
    class Scene;
    class SceneDelta;
    class SceneManager
    {
        // 允许Manager类访问SceneManager私有函数。
    public:
        void LoadScene();
        void UnloadScene();
        GameObject* CreateGameObject(const std::string& name);
        GameObject* FindGameObject(const std::string& name);

        void RemoveScene(const std::string& name);
        static void TickSimulation(uint32_t frameIndex);
        static void Create();
        static void Destroy();
        static void EndFrame();
        void Init();
    public:
        inline static SceneManager* GetInstance() 
        {
            if (!s_Instance) 
            {
                s_Instance = new SceneManager();
            }
            return s_Instance;
        }

        SceneManager();
        ~SceneManager();

        // todo： 这部分数据也要找地方存， maybe一个Global的渲染处

        Material* blitMaterial;
        Mesh* quadMesh;
        ResourceHandle<Shader> blitShader;
        ResourceHandle<Texture> testTexture;

        inline Scene* GetCurrentScene() 
        { 
            return mCurrentScene; 
        };
        inline void SetCurrentScene(Scene* scene) 
        { 
            mCurrentScene = scene; 
        };
        Scene* AddNewScene(const std::string& name);
        void SwitchSceneTo(const std::string& name);
        void SetCurrentFrame(uint32_t currentFrameIndex);

        SceneDelta FlushSceneDelta();
    private:
        static SceneManager* s_Instance;
        Scene* mCurrentScene = nullptr;
        std::unordered_map<std::string, Scene*> mSceneMap;
        std::vector<ResourceHandle<Texture>> texHandler;
    };

}
```

### File: `Runtime/Scene/CPUScene.h`
```cpp
namespace EngineCore
{
    class CPUScene
    {
    public:
        void Update(uint32_t frameID);
      
        void ApplyDirtyNode(uint32_t renderID, NodeDirtyFlags cpuWorldRenderNodeFlag , const NodeDirtyPayload& payload);
        void EndFrame();
        CPUSceneView GetSceneView();

        inline void SetCurrentFrame(uint32_t currentFrame)
        {   
            mCurrentFrame = currentFrame;
        }
    private:
        void EnsureCapacity(uint32_t renderID);
        void CreateRenderNode(uint32_t renderID, const NodeDirtyPayload& payload);
        void DeleteRenderNode(uint32_t renderID); 
        void OnRenderNodeMaterialDirty(uint32_t renderID, const NodeDirtyPayload& payload);
        void OnRenderNodeTransformDirty(uint32_t renderID, const NodeDirtyPayload& payload);
        void OnRenderNodeMeshDirty(uint32_t renderID, const NodeDirtyPayload& payload);
    
        
    private:
        vector<AssetID> materialList;
        vector<AssetID> meshList;
        vector<AABB> worldBoundsList;
        vector<AABB> localBoundCacheList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        uint32_t mCurrentFrame = 0;
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

### File: `Runtime/Renderer/RenderPath/LagacyRenderPipeline.h`
```cpp
namespace EngineCore
{
    class LagacyRenderPipeline : public IRenderPipeline
    {
    public:
        virtual ~LagacyRenderPipeline() override {};
        virtual void Prepare(RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
    
    };
```