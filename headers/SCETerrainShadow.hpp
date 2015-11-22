/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/******FILE:SCETerrainShadow.hpp*******/
/**************************************/
#ifndef SCE_TERRAINSHADOW_HPP
#define SCE_TERRAINSHADOW_HPP

#include "SCEDefines.hpp"

namespace SCE
{
    class SCE_GBuffer;

    class TerrainShadow
    {
    public :

        TerrainShadow();
        void RenderShadow(const mat4& projectionMatrix, const mat4& viewMatrix,
                          const vec3 &sunPosition, SCE_GBuffer &gbuffer, const glm::mat4 &worldToTerrainspace, uint terrainTexture, float heightScale);

    private :

        GLuint mShader;
        GLint  mWorldToTerrainMatUniform;
        GLint  mTerrainTexUniform;
        GLint  mPositionTexUniform;
        GLint  mFinalTexUniform;
        GLint  mSunPositionUniform;
        GLint  mHeightScaleUniform;

    };
}

#endif
