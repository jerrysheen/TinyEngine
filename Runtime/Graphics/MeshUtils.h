#pragma once
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Graphics/Mesh.h"

namespace EngineCore
{
    class MeshUtils
    {
    public:
        static void GetFullScreenQuad(Mesh* modelData);
    private:

    };
}