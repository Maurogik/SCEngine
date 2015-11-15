/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.hpp**********/
/**************************************/
#ifndef SCE_TERRAIN_HPP
#define SCE_TERRAIN_HPP

#include "SCEDefines.hpp"

//TODO make terrain textures and texture sizes customizable

namespace SCE
{
    namespace Terrain
    {
        void Init(float terrainSize, float patchSize, float terrainBaseHeight);
        void UpdateTerrain(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        void RenderTerrain(const mat4& projectionMatrix, const mat4& viewMatrix,
                           bool isShadowPass = false);
        float GetTerrainHeight(const vec3 &pos_worldspace);
        void Cleanup();
    }
}

#endif
