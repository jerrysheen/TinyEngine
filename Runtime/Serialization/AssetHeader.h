#pragma once
#include <iostream>

namespace EngineCore
{
    // type 0 = Scene
    // type 1 = Texture
    // type 2 = Mesh
    struct AssetHeader
    {
        char magic[4] = {'E', 'N', 'G', 'N'};
        uint32_t version = 1;
        uint32_t assetID = 0;
        uint32_t type = 0; 
    };
}