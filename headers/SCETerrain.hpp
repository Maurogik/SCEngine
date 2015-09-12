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
        void Init(float terrainSize = 500.0f, float patchSize = 20.0f);
        void RenderTerrain(const vec3& cameraPosition, const mat4& projectionMatrix, const mat4& viewMatrix, float terrainBaseHeight);
        void Cleanup();
    }
}

#endif
