#pragma once
#include "PreCompiledHeader.h"
#include "Manager.h"
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
#include "GameObject/Transform.h"
#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
#include "Resources/MetaFile.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceHandle.h"

namespace EngineCore
{
    class SceneManager : public Manager<SceneManager>
    {
        // 允许Manager类访问SceneManager私有函数。
        friend class Manager<SceneManager>;
    public:
        void LoadScene();
        void UnloadScene();
        static void Update();
        static void Create();
    public:
        SceneManager();
        ~SceneManager();

        static GameObject* mTestGameObject;
        
        GameObject* mainCameraGo;
        ResourceHandle<ModelData> testMesh;
        //MaterialMetaData* testMatMetaData;
        ResourceHandle <Material> testMat;
        ResourceHandle<Shader> testShader;

        ResourceHandle<Material> blitMaterial;
        ResourceHandle<ModelData> quadMesh;
        ResourceHandle<Shader> blitShader;

        ResourceHandle<Texture> testTexture;
    };

}