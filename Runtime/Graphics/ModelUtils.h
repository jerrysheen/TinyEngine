#pragma once
#include "ModelData.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Graphics/ModelData.h"

namespace EngineCore
{
    class ModelUtils
    {
    public:
        static void GetFullScreenQuad(ModelData* modelData);
    private:

    };
}