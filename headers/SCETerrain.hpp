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
    class SCE_GBuffer;
    namespace Terrain
    {
        void Init(float terrainSize, float patchSize, float terrainBaseHeight,
                  int nbRepeat, float maxTessDist = 3000.0f);

        void UpdateTerrain(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);

        void RenderTerrain(const mat4& projectionMatrix, const mat4& viewMatrix,
                           bool isShadowPass = false);

        void RenderShadow(const mat4& projectionMatrix, const mat4& viewMatrix,
                          const vec3 &sunPosition, SCE_GBuffer &gbuffer);

        void RenderTrees(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix,
                         bool isShadowPass = false);

        float GetTerrainHeight(const vec3 &pos_worldspace);

        glm::vec3 GetTerrainNormal(const vec3& pos_worldspace);

        void Cleanup();
    }
}

#endif
