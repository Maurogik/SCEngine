/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.hpp**********/
/**************************************/
#ifndef SCE_TERRAIN_HPP
#define SCE_TERRAIN_HPP

#include "SCEDefines.hpp"

//TODO make terrain textures and texture sizes customizable
//TODO make terrain component or access from scene
//TODO have terrain shadows

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
