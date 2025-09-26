#pragma once
#include "PreCompiledHeader.h"
#include "Manager.h"
#include "Core/GameObject.h"
#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
#include "Core/Resources.h"
#include "Graphics/Shader.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"

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

        ModelData* testMesh;
        MaterialStruct* testMatStruct;
        Material* testMat;
        Shader* testShader;
        Camera* mCamera;

        Material* blitMaterial;
        ModelData* quadMesh;
        Shader* blitShader;

        TextureStruct* testTextureMeta;
        Texture* testTexture;
    };

}