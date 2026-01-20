#pragma once
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Serialization/MetaData.h"
#include "Resources/Resource.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Math/AABB.h"

namespace EngineCore
{
    // 用来描述model Input 或者 shader reflection input
    struct InputLayout
    {
        VertexAttribute type;
        int size;
        int dimension;
        int stride;
        int offset;
        InputLayout(VertexAttribute _type, int _size, int _dimension, int _stride, int _offset)
        {
            type = _type; size = _size; dimension = _dimension; stride = _stride; offset = _offset;
        };
        InputLayout() = default;
        InputLayout(VertexAttribute type) : type(type) {};
    };


    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
    };

    struct MeshBufferAllocation
    {
        IGPUBuffer* buffer = nullptr;
        // 当前数据开始位置， 可以直接绑定
        uint64_t gpuAddress = 0;
        uint64_t offset =0;
        uint64_t size = 0;
        uint32_t stride = 0;
        bool isValid = false;
        struct MeshBufferAllocation() = default;
        struct MeshBufferAllocation(IGPUBuffer* buffer, uint64_t gpuAddress, uint64_t offset, uint64_t size, uint64_t stride)
            :buffer(buffer), gpuAddress(gpuAddress), offset(offset), size(size), stride(stride)
        {
            isValid = true;
        }
    };

    class Mesh : public Resource
    {
    public:
        // todo: 先这么写，后续或许抽成单独Component

        Mesh() = default;
        Mesh(MetaData* metaData);
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        bool isDynamic = true;
        virtual void OnLoadComplete() override { UploadMeshToGPU(); };
    private:
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);

    };

}