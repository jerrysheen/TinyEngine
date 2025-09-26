#include "PreCompiledHeader.h"
#include "SceneManager.h"
#include "Graphics/Texture.h"

namespace EngineCore
{
    std::unique_ptr<SceneManager> SceneManager::s_Instance = nullptr;
    GameObject* SceneManager::mTestGameObject = nullptr;
    
    SceneManager::SceneManager()
    {
        // loadtest Texture
        testTextureMeta = Resources::LoadTextureMeta("");
        testTexture = Texture::LoadTexture(testTextureMeta);

        //std::cout << "Init Scene Manager!!" << std::endl;
        mTestGameObject = new GameObject();
        mTestGameObject->AddComponent<MeshFilterComponent>();
        testMesh = ModelUtils::LoadMesh("D:/GitHubST/TinyEngine/Assets/Model/cube.obj");
        
        
        testMatStruct = Resources::LoadMaterialMeta("D:/GitHubST/TinyEngine/Assets/Model/cube.obj");
        testMat = new Material(testMatStruct);
        testShader = Shader::Compile("D:/GitHubST/TinyEngine/Assets/Shader/SimpleTestShader.hlsl");
        testMat->shader = testShader;
        testMat->SetUpGPUResources();
        testMat->SetTexture("DiffuseTexture", Texture("Material"));

        mCamera = new Camera();

        quadMesh = ModelUtils::GetFullScreenQuad();
        blitShader = Shader::Compile("D:/GitHubST/TinyEngine/Assets/Shader/BlitShader.hlsl");
        blitMaterial = new Material();
        blitMaterial->shader = blitShader;
        blitMaterial->SetUpGPUResources();
        blitMaterial->SetFloat("_FlipY", 1.0f);
        blitMaterial->SetTexture("SrcTexture", Texture("CameraColorAttachment"));


    }

    SceneManager::~SceneManager()
    {
        delete mTestGameObject;
        mTestGameObject = nullptr;
    };

    void SceneManager::Update()
    {
        auto meshFilterComponent = mTestGameObject->GetComponent<MeshFilterComponent>();
    }

    void SceneManager::Create()
    {
        SceneManager::s_Instance = std::make_unique<SceneManager>();
    }
}