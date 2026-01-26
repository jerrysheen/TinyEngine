# Architecture Digest: SERIALIZATION
> Auto-generated. Focus: AssetSerialization, BaseTypeSerialization, ComponentFactory, JsonSerializer, MetaData, MetaFactory, MetaLoader, MeshRenderer, MeshFilter, Material, ModelData

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Key Files Index
- `[32]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[27]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[27]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[27]` **Runtime/Graphics/MaterialInstance.h** *(Content Included)*
- `[27]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[26]` **Runtime/Graphics/MaterialData.h** *(Content Included)*
- `[16]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[16]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[11]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[10]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
- `[9]` **Runtime/Scene/BistroSceneLoader.h**
- `[9]` **Editor/Panel/EditorMainBar.h**
- `[8]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Graphics/GPUSceneManager.h**
- `[7]` **Runtime/Renderer/RenderAPI.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[7]` **Runtime/Renderer/RenderStruct.h**
- `[6]` **Runtime/Scene/SceneManager.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[5]` **Runtime/Renderer/RenderSorter.h**
- `[5]` **Runtime/Resources/AssetTypeTraits.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[5]` **Assets/Shader/include/Core.hlsl**
- `[4]` **Runtime/GameObject/ComponentType.h**
- `[4]` **Runtime/GameObject/GameObject.h**
- `[3]` **Runtime/Graphics/MeshUtils.h**
- `[3]` **Runtime/Resources/Asset.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/GameObject/Camera.h**
- `[2]` **Runtime/GameObject/Component.h**
- `[2]` **Runtime/GameObject/MonoBehaviour.h**
- `[2]` **Runtime/GameObject/Transform.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/GeometryManager.h**
- `[2]` **Runtime/Graphics/GPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/GPUTexture.h**
- `[2]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[2]` **Runtime/Graphics/IGPUResource.h**
- `[2]` **Runtime/Graphics/Mesh.h**
- `[2]` **Runtime/Graphics/RenderTexture.h**
- `[2]` **Runtime/Graphics/Shader.h**
- `[2]` **Runtime/Graphics/Texture.h**
- `[2]` **Runtime/Managers/Manager.h**
- `[2]` **Runtime/Managers/WindowManager.h**
- `[2]` **Runtime/Math/AABB.h**
- `[2]` **Runtime/Math/Frustum.h**
- `[2]` **Runtime/Math/Math.h**
- `[2]` **Runtime/Math/Matrix4x4.h**
- `[2]` **Runtime/Math/Plane.h**

## Evidence & Implementation Details

### File: `Runtime/GameObject/MeshRenderer.h`
```cpp
namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override;
        static ComponentType GetStaticType() { return ComponentType::MeshRenderer; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshRenderer; };

        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        
        void SetUpMaterialPropertyBlock();

        inline Material* GetSharedMaterial()
        { 
            return mShardMatHandler.IsValid() ? mShardMatHandler.Get() : nullptr;
        };

        inline void SetSharedMaterial(const ResourceHandle<Material>& mat) 
        {
            mShardMatHandler = mat;
            SetUpMaterialPropertyBlock();
        }

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);
        uint32_t lastSyncTransformVersion = 0;
        bool shouldUpdateMeshRenderer = true;

        AABB worldBounds;
        uint32_t sceneRenderNodeIndex = UINT32_MAX;
        bool materialDirty = true;
		
        void TryAddtoBatchManager();

        uint32_t renderLayer = 1;
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

    };
```

### File: `Runtime/GameObject/MeshFilter.h`
```cpp
namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);

        virtual ~MeshFilter() override;
        static ComponentType GetStaticType() { return ComponentType::MeshFilter; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshFilter; };
        void OnLoadResourceFinished();
    public:
        ResourceHandle<Mesh> mMeshHandle;
        
        virtual const char* GetScriptName() const override { return "MeshFilter"; }

        uint32_t GetHash()
        {
            return mMeshHandle->GetInstanceID();
        }
    private:
        uint32_t hash;
    };
```

### File: `Runtime/Graphics/Material.h`
```cpp
namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;
        
        MaterialData m_MaterialData;
        void LoadFromMaterialData(const MaterialData& data);

        Material() = default;
        Material(ResourceHandle<Shader> shader);
        Material(const Material& other);
        void UploadDataToGpu();
        ~Material();


        void SetValue(const string& name, void* data, uint32_t size) 
        {
            ASSERT(matInstance != nullptr);
            matInstance->SetValue(name, data, size);
        }

        // 通用设置材质texture的接口
        void SetTexture(const string& name, IGPUTexture* texture)
        {
            ASSERT(textureData.count(name) > 0);
            if(textureHandleMap.count(name))
            {
                textureHandleMap.erase(name);
            }
            textureData[name] = texture;
        }

        // 运行时关联一个临时资源，建立一个引用， 防止资源因为0引用被销毁
        void SetTexture(const string& name, ResourceHandle<Texture> texture)
        {
            ASSERT(textureData.count(name) > 0);
            textureHandleMap[name] = texture;
            if(texture.IsValid())
            {
                textureData[name] = texture.Get()->textureBuffer;
            }
        }

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
```

### File: `Runtime/Graphics/MaterialInstance.h`
```cpp
namespace EngineCore
{
    class MaterialInstance
    {
    public:
        MaterialInstance(const MaterialLayout& layout)
            :m_Layout(layout)
        {
            m_DataBlob.resize(m_Layout.GetSize());    
        }
        
        void SetValue(const std::string& name, void* data, uint32_t size)
        {
            uint32_t offset = m_Layout.GetPropertyOffset(name);
            memcpy(m_DataBlob.data() + offset, data, size);
        }

        void* GetData(){return m_DataBlob.data();}
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
    private:
        unordered_map<std::string, IGPUTexture*> textureMap;
        MaterialLayout m_Layout;
        std::vector<uint8_t> m_DataBlob;
        bool m_Dirty = true;
    };
```

### File: `Runtime/Graphics/MaterialLayout.h`
```cpp
namespace EngineCore
{
    struct MaterialPropertyLayout
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        ShaderVariableType type;
    };
```
...
```cpp
    public:
        // 后续用json加载
        void BuildFromJson(){};
```
...
```cpp
            // --- Chunk 0 ---
            // float4 DiffuseColor
            AddProp("DiffuseColor", ShaderVariableType::VECTOR4, 16);
            
            // float4 SpecularColor
            AddProp("SpecularColor", ShaderVariableType::VECTOR4, 16);

            // float Roughness
            AddProp("Roughness", ShaderVariableType::FLOAT, 4);
            
            // float Metallic
            AddProp("Metallic", ShaderVariableType::FLOAT, 4);

            // float2 TilingFactor (8 bytes)
            AddProp("TilingFactor", ShaderVariableType::VECTOR2, 8);
            
            AddProp("DiffuseTextureIndex", ShaderVariableType::FLOAT, 4);
            
            AddProp("PaddingLast", ShaderVariableType::VECTOR3, 12);

            // 此时 offset = 16+16+4+4+8 = 48 bytes
            // 还需要补齐到 16 字节对齐吗？HLSL cbuffer 是 16 字节对齐的
            // 目前 48 刚好是 16 的倍数，完美。
            
            layout.m_TotalSize = currentOffset;
            return layout;
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
```
...
```cpp
        };

        uint32_t GetSize(){ return m_TotalSize;}
```

### File: `Runtime/Graphics/MaterialData.h`
```cpp
    // 对应 MaterialLayout::GetDefaultPBRLayout 的内存布局
    // 必须保持 16 字节对齐
    struct MaterialConstants
    {
        Vector4 DiffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 SpecularColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        float Roughness = 0.5f;
        float Metallic = 0.0f;
        Vector2 TilingFactor = Vector2(1.0f, 1.0f);
        float DiffuseTextureIndex = 0.0f; 
        Vector3 Padding = Vector3(0.0f, 0.0f, 0.0f);
    };
```

### File: `Runtime/Renderer/BatchManager.h`
```cpp
        Mesh* mesh;
        DrawIndirectContext() = default;
        DrawIndirectContext(Material* mat, Mesh* mesh): material(mat), mesh(mesh){}
```
...
```cpp
        }

        void TryAddBatchCount(MeshRenderer* meshRenderer);
        void TryDecreaseBatchCount(MeshRenderer* meshRenderer);

        void TryAddBatchCount(MeshFilter* meshFilter);
        void TryDecreaseBatchCount(MeshFilter* meshFilter);
        static std::unordered_map<uint64_t, int> BatchMap;
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, MeshFilter* meshFilter, uint32_t layer); 

        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        void TryAddBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        void TryDecreaseBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static void Create();
        static BatchManager* s_Instance;

    };
}
```

### File: `Runtime/Scene/Scene.h`
```cpp
#include "Renderer/RenderStruct.h"

namespace EngineCore
{
    struct RenderSceneData
    {
        vector<MeshRenderer*> meshRendererList;
        vector<MeshFilter*> meshFilterList;
        vector<AABB> aabbList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        // 每帧重置
        vector<uint32_t> transformDirtyList;
        vector<uint32_t> materialDirtyList;
        struct RenderSceneData() = default;
        inline void SyncData(MeshRenderer* renderer, uint32_t index)
        {
            meshRendererList[index] = renderer;
            if (renderer && renderer->gameObject)
            {
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
                auto* meshFilter = renderer->gameObject->GetComponent<MeshFilter>();
                if(meshFilter != nullptr)
                {
                    meshFilterList[index] = meshFilter;
                }
                else
                {
                    meshFilterList[index] = nullptr;
                }
            }
        }

        inline void PushNewData() 
        {
            meshRendererList.emplace_back();
            meshFilterList.emplace_back();
            aabbList.emplace_back();
            objectToWorldMatrixList.emplace_back();
            layerList.emplace_back();
        }


        inline void DeleteData(uint32_t index)
        {
            meshRendererList[index] = nullptr;
            meshFilterList[index] = nullptr;

            // 后续删除RenderProxy
        }

        inline void ClearDirtyList()
        {
            materialDirtyList.clear();
            transformDirtyList.clear();
        }

        inline void UpdateDirtyRenderNode()
```
...
```cpp
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
```
...
```cpp
        void Close(){};
        void Update();
        void EndFrame();
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void Scene::DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
```
...
```cpp
        // todo:
        // 材质更新的时候， 也需要去调用这个逻辑，比如changeRenderNodeInfo之类的，
        int AddNewRenderNodeToCurrentScene(MeshRenderer* renderer);
        void DeleteRenderNodeFromCurrentScene(uint32_t index);
        
    public:
        int m_CurrentSceneRenderNodeIndex = 0;
        std::queue<uint32_t> m_FreeSceneNode;
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;
        RenderSceneData renderSceneData;
    private:
    };    
} // namespace EngineCore
```

### File: `Runtime/Core/PublicStruct.h`
```cpp

    // constantbuffer中的变量记录
    struct ShaderConstantInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };
```
...
```cpp


    struct ShaderReflectionInfo
    {
        RootSignatureKey mRootSigKey;

        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mConstantBufferInfo;
        vector<ShaderBindingInfo > mTextureInfo;
        vector<ShaderBindingInfo > mSamplerInfo;
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};


        // 定义偏移量常量 (方便修改)
        static const int BIT_OFFSET_CBV = 0;
        static const int BIT_OFFSET_SRV = 16;
        static const int BIT_OFFSET_UAV = 48;
        static const int BIT_OFFSET_SAMPLER = 56;
    };
```
...
```cpp
    // 前向声明，防止循环引用。
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        MeshRenderer* meshRenderer;
        MeshFilter* meshFilter;
        float distanToCamera = 0;
    };
```
...
```cpp


    struct DrawRecord
    {
        Material* mat;
        Mesh* mesh;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, Mesh* mesh)
            :mat(mat), mesh(mesh), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, Mesh* mesh, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), mesh(mesh), perDrawHandle(handle), instanceCount(instCount){}
    };
```
...
```cpp
    };

    class RenderPass;
    struct RenderPassAsset
    {
        vector<RenderPass*> renderPasses;
        inline void Clear()
        {
             for (RenderPass* pass : renderPasses) 
                delete pass;
        };
    };
```

### File: `Runtime/Serialization/SceneLoader.h`
```cpp
#include "Resources/AssetRegistry.h"
#include "Settings/ProjectSettings.h"
namespace EngineCore
{
    struct SceneSerializedNode
    {
        char name[64];         // 固定长度名字
        int32_t parentIndex = -1;
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;

        uint64_t meshID = 0;
        uint32_t materialID = 0;

    };

    class SceneLoader: public IResourceLoader                       
    {
    public:
        virtual ~SceneLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);

            // todo: 确保进来的文件是Scene类型
            in.seekg(sizeof(AssetHeader));

            Scene* scene = new Scene();
            std::vector<SceneSerializedNode> allnode;
            StreamHelper::ReadVector(in, allnode);
            std::unordered_map<uint32_t, GameObject*> gameObjectMap;
            
            for(int i = 0; i < allnode.size(); i++)
            {
                auto& nodeData = allnode[i];
                
                std::string nodeName = nodeData.name;
                GameObject* go = scene->CreateGameObject(nodeName.empty() ? "Node" : nodeName);

                go->transform->SetLocalPosition(nodeData.position);
                go->transform->SetLocalQuaternion(nodeData.rotation);
                go->transform->SetLocalScale(nodeData.scale);
                if(nodeData.parentIndex != -1)
                {
                    ASSERT(gameObjectMap.count(nodeData.parentIndex) > 0);
                    go->SetParent(gameObjectMap[nodeData.parentIndex]);
                }
                gameObjectMap[i] = go;

                //todo 加入材质的异步加载：
                if(nodeData.meshID != 0)
                {
                    MeshFilter* filter = go->AddComponent<MeshFilter>();
                    filter->mMeshHandle = ResourceManager::GetInstance()->LoadAssetAsync<Mesh>(nodeData.meshID, [=]() 
                        {
                            filter->OnLoadResourceFinished();
                        });
                }
```
...
```cpp
        void SaveSceneToBin(const Scene* scene, const std::string& relativePath, uint32_t id)
        {
            ASSERT(scene && scene->allObjList.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 0;
            StreamHelper::Write(out, header);

            std::vector<SceneSerializedNode> linearNode;
            std::unordered_map<GameObject*, uint32_t> gameObjectMap;
            for(int i = 0; i < scene->rootObjList.size(); i++)
            {
                GameObject* gameObject = scene->rootObjList[i];
                
                SerilizedNode(gameObject, gameObjectMap, linearNode);
            }
```
...
```cpp

            // Name Serialization
            memset(node.name, 0, sizeof(node.name));
            const std::string& goName = gameObject->name;
            size_t copyLen = std::min(goName.size(), sizeof(node.name) - 1);
            memcpy(node.name, goName.c_str(), copyLen);

            Transform* parent = gameObject->transform->parentTransform;
            uint64_t meshID = 0;
            MeshFilter* meshfilter = gameObject->GetComponent<MeshFilter>();
            if (meshfilter != nullptr)
            {
                meshID = meshfilter->mMeshHandle->GetAssetID();
            }
```
...
```cpp
            else
            {
                ASSERT(gameObjectMap.count(parent->gameObject) > 0);
                node.parentIndex = gameObjectMap[parent->gameObject];
                gameObjectMap[gameObject] = linearNode.size();
            }
            node.materialID = 0;
            node.meshID = meshID;
            node.position = gameObject->transform->GetLocalPosition();
            node.rotation = gameObject->transform->GetLocalQuaternion();
            node.scale = gameObject->transform->GetLocalScale();
            linearNode.push_back(node);

            for (auto* child : gameObject->GetChildren()) 
            {
```