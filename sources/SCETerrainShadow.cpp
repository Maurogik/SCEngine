/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/******FILE:SCETerrainShadow.cpp*******/
/**************************************/

#include "../headers/SCETerrainShadow.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCE_GBuffer.hpp"
#include "../headers/SCERender.hpp"

SCE::TerrainShadow::TerrainShadow()
{
    //Setup shadow data
    mShader = SCE::ShaderUtils::CreateShaderProgram("Terrain/ShadowPass");
    mTerrainTexUniform =
            glGetUniformLocation(mShader, "TerrainHeightMap");
    mPositionTexUniform =
            glGetUniformLocation(mShader, "PositionTex");
    mWorldToTerrainMatUniform =
            glGetUniformLocation(mShader, "WorldToTerrainSpace");
    mSunPositionUniform =
            glGetUniformLocation(mShader, "SunPosition_worldspace");
    mFinalTexUniform =
            glGetUniformLocation(mShader, "FinalTex");
    mHeightScaleUniform =
            glGetUniformLocation(mShader, "HeightScale");
}

void SCE::TerrainShadow::RenderShadow(const mat4 &projectionMatrix, const mat4 &viewMatrix,
                                      const vec3 &sunPosition, SCE_GBuffer &gbuffer,
                                      const mat4 &worldToTerrainspace, uint terrainTexture,
                                      float heightScale)
{
    glUseProgram(mShader);

    gbuffer.SetupTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION,
                         mPositionTexUniform, 0);
    gbuffer.SetupFinalTexture(mFinalTexUniform, 1);

    //set render target to GBuffer final tex
    gbuffer.BindForLightPass();

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);
    glUniform1i(mTerrainTexUniform, 2);

    glUniform1f(mHeightScaleUniform, heightScale);
    glUniform3fv(mSunPositionUniform, 1, &sunPosition[0]);
    glUniformMatrix4fv(mWorldToTerrainMatUniform, 1, GL_FALSE,
                       &(worldToTerrainspace[0][0]));

    SCE::SCERender::RenderFullScreenPass(mShader, projectionMatrix, viewMatrix);
}
