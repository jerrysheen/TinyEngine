#pragma once
#include "PreCompiledHeader.h"
#include "Manager.h"
#include "Core/GameObject.h"
#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
#include "Resources/MetaFile.h"
#include "Graphics/Shader.h"
#include "Graphics/Camera.h"
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

        ResourceHandle<ModelData> testMesh;
        //MaterialMetaData* testMatMetaData;
        ResourceHandle <Material> testMat;
        ResourceHandle<Shader> testShader;
        Camera* mCamera;

        ResourceHandle<Material> blitMaterial;
        ResourceHandle<ModelData> quadMesh;
        ResourceHandle<Shader> blitShader;

        ResourceHandle<Texture> testTexture;
    };

}