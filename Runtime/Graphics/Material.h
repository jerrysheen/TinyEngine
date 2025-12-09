#pragma once
#include "Math.h"
#include "MaterialData.h"
#include "Serialization/MetaData.h"
#include "Shader.h"
#include "Buffer.h"
#include "Core/Object.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderCommand.h"
#include "Resources/ResourceHandle.h"
#include <variant>
#include "Graphics/FrameBufferObject.h"
#include "Graphics/Texture.h"
#include <tuple>

namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        MaterialData mMaterialdata;

        // 真正持有的资源句柄
        ResourceHandle<Shader> mShader;
        // 这个地方应该限定只有 Texture 和 FrameBufferObject能过来
        unordered_map<std::string, std::variant<ResourceHandle<Texture>, ResourceHandle<FrameBufferObject>>> mTexResourceMap;


        Material() = default;
        Material(MetaData* metaData);
        Material(ResourceHandle<Shader> shader);
        Material(const Material& other);
        void SetUpGPUResources();
        void UploadDataToGpu();
        ~Material();

        void SetTexture(const string& name, uint64_t texInstanceID);
        void SetTexture(const string& name, ResourceHandle<Texture> handle);
        void SetTexture(const string& name, ResourceHandle<FrameBufferObject> handle);
        void SetMatrix4x4(const string& name, const Matrix4x4& matrix4x4);
        void SetFloat(const string& name, float value);

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        // todo  自己同步材质；
        MaterailRenderState mRenderState;
    private:
        void LoadDependency(const std::unordered_map<std::string, MetaData>& dependentMap);
        void SetUpRenderState();
        void GetMaterialDataFromShaderReflection();
    };
}