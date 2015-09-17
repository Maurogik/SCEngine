/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.hpp**********/
/**************************************/
#ifndef SCE_TERRAIN_HPP
#define SCE_TERRAIN_HPP

#include "SCEDefines.hpp"

namespace SCE
{
    namespace Terrain
    {
        void Init(float terrainSize, float patchSize, float terrainBaseHeight);
        void RenderTerrain(const mat4& projectionMatrix, const mat4& viewMatrix,
                           float tesselationOverride = -1.0f);
        void Cleanup();
    }
}

#endif
