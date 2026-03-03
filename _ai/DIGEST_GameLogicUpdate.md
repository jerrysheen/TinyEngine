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
- `[100]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp** *(Content Included)*
- `[94]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h** *(Content Included)*
- `[91]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[81]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[69]` **Runtime/Scene/GPUScene.cpp** *(Content Included)*
- `[68]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[68]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[65]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[64]` **Runtime/Scene/GPUScene.h** *(Content Included)*
- `[64]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp** *(Content Included)*
- `[60]` **Runtime/Scene/SceneManager.h** *(Content Included)*
- `[56]` **Runtime/Scene/CPUScene.cpp** *(Content Included)*
- `[56]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[55]` **Runtime/Scene/CPUScene.h** *(Content Included)*
- `[52]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[51]` **Runtime/Renderer/FrameContext.cpp** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h** *(Content Included)*
- `[49]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h** *(Content Included)*
- `[47]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[47]` **Runtime/Renderer/FrameContext.h**
- `[45]` **Runtime/Renderer/RenderPath/LagacyRenderPath.cpp**
- `[45]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[42]` **Runtime/Renderer/Renderer.cpp**
- `[42]` **Runtime/Renderer/Renderer.h**
- `[40]` **Runtime/GameObject/MeshRenderer.h**
- `[39]` **Runtime/Scene/Scene.h**
- `[38]` **Runtime/Core/Game.h**
- `[38]` **Runtime/Renderer/RenderContext.h**
- `[38]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[38]` **Runtime/Scene/Scene.cpp**
- `[38]` **Runtime/Serialization/SceneLoader.h**
- `[33]` **Runtime/GameObject/Camera.cpp**
- `[33]` **Runtime/GameObject/GameObject.h**
- `[33]` **Runtime/Scene/BistroSceneLoader.h**
- `[32]` **Runtime/GameObject/GameObject.cpp**
- `[32]` **Runtime/Scene/SceneStruct.h**
- `[30]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[29]` **Runtime/Renderer/Culling.cpp**
- `[28]` **Editor/Panel/EditorGameViewPanel.cpp**
- `[25]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp**
- `[24]` **Runtime/Entry.cpp**
- `[24]` **Editor/Panel/EditorGameViewPanel.h**
- `[22]` **Runtime/GameObject/MeshFilter.cpp**
- `[22]` **Runtime/Renderer/RenderCommand.h**
- `[20]` **Runtime/GameObject/Transform.cpp**
- `[19]` **Runtime/GameObject/Transform.h**
- `[19]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[18]` **Runtime/Graphics/Material.cpp**
- `[18]` **Runtime/Renderer/BatchManager.h**
- `[18]` **Runtime/Renderer/RenderAPI.h**
- `[18]` **Editor/Panel/EditorMainBar.cpp**
- `[17]` **Runtime/Renderer/RenderPath/IRenderPath.h**
- `[16]` **Runtime/Renderer/RenderSorter.h**
- `[15]` **Runtime/GameObject/Component.h**
- `[15]` **Runtime/Renderer/Culling.h**
- `[14]` **Runtime/GameObject/MeshFilter.h**
- `[14]` **Runtime/GameObject/MonoBehaviour.h**
- `[13]` **Runtime/GameObject/Camera.h**
- `[13]` **Runtime/GameObject/ComponentType.h**

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

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp`
```cpp
        //todo:
        // 这个地方要把ResourceState切换一下
        Renderer::GetInstance()->BeginFrame();

        FrameContext* currentFrame = RenderEngine::GetInstance()->GetGPUScene().GetCurrentFrameContexts();
        auto* visibilityBuffer = currentFrame->visibilityBuffer;
        Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
        Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);


        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
        int gameObjectCount = SceneManager::GetInstance()->GetCurrentScene()->allObjList.size();

        GPUCullingParam cullingParam;
        cullingParam.frustum = cam->mFrustum;
        cullingParam.totalItem = gameObjectCount;
        cullingParamBuffer->UploadBuffer(cullingParamAlloc, &cullingParam, sizeof(GPUCullingParam));

        
        // Get Current BatchInfo:
        vector<DrawIndirectArgs> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
        if (batchInfo.size() != 0) 
        {
            indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
        }
```
...
```cpp
        payload.groupY = 1;
        payload.groupZ = 1;
        Renderer::GetInstance()->DispatchComputeShader(payload);
        // 先把shader跑起来， 渲染到RT上， 后续blit啥的接入后面再说

        // 这个地方简单跑个绑定测试？
        Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_SHADER_RESOURCE);
        Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_INDIRECT_ARGUMENT);

        context.Reset();
        context.camera = cam;
        Renderer::GetInstance()->Render(context);
#ifdef EDITOR
        Renderer::GetInstance()->OnDrawGUI();
#endif

        Renderer::GetInstance()->EndFrame();
    }
}
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

### File: `Runtime/Scene/GPUScene.h`
```cpp
{

    class GPUScene
    {
    public:
        GPUScene();
        void Create();
        void Update(uint32_t currentFrameIndex);
        void Destroy();
        void EndFrame();
        
        BufferAllocation GetSinglePerMaterialData();
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void UpdateDirtyNode(CPUSceneView& view);
        void UploadCopyOp();

        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);


        void UpdateFrameContextDirtyFlags(uint32_t renderID, uint32_t flag);
        void UpdateFrameContextShadowData(uint32_t renderID, CPUSceneView& view);
        
        FrameContext* GetCurrentFrameContexts();
        inline uint32_t GetCurrentFrameID() const { return mCurrentFrameID; }

        inline GPUBufferAllocator* GetAllMaterialDataBuffer() { return allMaterialDataBuffer; }
        inline ResourceHandle<ComputeShader> GetCullingShaderHandler() { return GPUCullingShaderHandler; }

        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCurrentFrameID = currentFrame;
        }
    private:
        void EnsureCapacity(uint32_t renderID);
    private:
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
        static const int mMaxFrameCount = 3;
        uint32_t mCurrentFrameID = 0;
        FrameContext* mCurrentFrameContext;
        FrameContext mCPUFrameContext[mMaxFrameCount];
        GPUBufferAllocator* allMaterialDataBuffer;
    };
```

### File: `Runtime/Scene/SceneManager.h`
```cpp
namespace EngineCore
{
    class Scene;
    class SceneManager
    {
        // 允许Manager类访问SceneManager私有函数。
    public:
        void LoadScene();
        void UnloadScene();
        GameObject* CreateGameObject(const std::string& name);
        GameObject* FindGameObject(const std::string& name);

        void RemoveScene(const std::string& name);
        static void Update(uint32_t frameIndex);
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
    private:
        static SceneManager* s_Instance;
        Scene* mCurrentScene = nullptr;
        unordered_map<std::string, Scene*> mSceneMap;
        vector<ResourceHandle<Texture>> texHandler;
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
      
        void ApplyDirtyNode(uint32_t renderID, NodeDirtyFlags cpuWorldRenderNodeFlag , NodeDirtyPayload& payload);
        void EndFrame();
        CPUSceneView GetSceneView();

        inline void SetCurrentFrame(uint32_t currentFrame)
        {   
            mCurrentFrame = currentFrame;
        }
    private:
        void EnsureCapacity(uint32_t renderID);
        void CreateRenderNode(uint32_t renderID, NodeDirtyPayload& payload);
        void DeleteRenderNode(uint32_t renderID); 
        void OnRenderNodeMaterialDirty(uint32_t renderID, NodeDirtyPayload& payload);
        void OnRenderNodeTransformDirty(uint32_t renderID, NodeDirtyPayload& payload);
        void OnRenderNodeMeshDirty(uint32_t renderID, NodeDirtyPayload& payload);
    
        
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