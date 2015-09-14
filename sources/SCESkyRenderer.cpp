/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCESkyRenderer.cpp********/
/**************************************/

#include "../headers/SCESkyRenderer.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCE_GBuffer.hpp"


#define SUN_SHADER_NAME "SunShader"
#define SKY_SHADER_NAME "SkyShader"
#define SUN_POS_UNIFORM_NAME "SunPosition_worldspace"
#define QUALITY_UNIFORM_NAME "SizeQuality"

#define SUN_TEXTURE_QUALITY 0.25f

namespace SCE
{

namespace SkyRenderer
{

    struct SkyData
    {
        SkyData()
            : sunShaftTexture(-1), sunShaftProgram(-1), qualityUniform(-1), skyProgram(-1),
              sunPositionUniform(), fboId(-1)
        {}

        //sun shader data
        GLuint  sunShaftTexture;
        GLuint  sunShaftProgram;
        GLint   qualityUniform;
        //sky shader data
        GLuint  skyProgram;
        //common data
        GLint   sunPositionUniform[2]; //{unif for sun shader, unif for sky shader}
        GLuint  fboId;
        uint    renderWidth;
        uint    renderHeight;
    };

    SkyData skyData;

    void Init(uint windowWidth, uint windowHeight)
    {
        float quality = SUN_TEXTURE_QUALITY;

        skyData.renderWidth = float(windowWidth) * quality;
        skyData.renderHeight = float(windowHeight) * quality;

        // Create the FBO
        glGenFramebuffers(1, &(skyData.fboId));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, skyData.fboId);

        glGenTextures(1, &(skyData.sunShaftTexture));
        //texture will store additive sun color as RGB and fog strength as A
        glBindTexture(GL_TEXTURE_2D, skyData.sunShaftTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, skyData.renderWidth, skyData.renderHeight,
                     0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, skyData.sunShaftTexture, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::RaiseError("FrameBuffer creation error, status:" + std::to_string(status));
        }

        // restore default FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        skyData.skyProgram = SCE::ShaderUtils::CreateShaderProgram(SKY_SHADER_NAME);
        skyData.sunShaftProgram = SCE::ShaderUtils::CreateShaderProgram(SUN_SHADER_NAME);
        skyData.qualityUniform = glGetUniformLocation(skyData.sunShaftProgram, QUALITY_UNIFORM_NAME);
        skyData.sunPositionUniform[0] = glGetUniformLocation(skyData.sunShaftProgram,SUN_POS_UNIFORM_NAME);
        skyData.sunPositionUniform[1] = glGetUniformLocation(skyData.skyProgram, SUN_POS_UNIFORM_NAME);
    }

    void Render(const SCE::CameraRenderData& renderData, SCE_GBuffer& gBuffer,
                const glm::vec3& sunPosition)
    {
        GLint viewportDims[4];
        glGetIntegerv( GL_VIEWPORT, viewportDims );

        //only a screen space quad, don't need depth testing
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glCullFace(GL_FRONT);

        //Renders sun shaft in smaller res
        glViewport(0, 0, skyData.renderWidth, skyData.renderHeight);
        glUseProgram(skyData.sunShaftProgram);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, skyData.fboId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);
        gBuffer.BindTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION,
                            0, 0);
        glUniform1f(skyData.qualityUniform, SUN_TEXTURE_QUALITY);
        glUniform3f(skyData.sunPositionUniform[0], sunPosition.x, sunPosition.y, sunPosition.z);

        //Render sun pass to texture
        SCERender::RenderFullScreenPass(skyData.sunShaftProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);
        //restore viewport
        glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);

        //Render sky
        glUseProgram(skyData.skyProgram);

        gBuffer.BindForSkyPass();
        //bind sun texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, skyData.sunShaftTexture);
        glUniform1i(2, 2);

        glUniform3f(skyData.sunPositionUniform[1], sunPosition.x, sunPosition.y, sunPosition.z);

        SCERender::RenderFullScreenPass(skyData.skyProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);

        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    void Cleanup()
    {
        if(skyData.sunShaftTexture != GLuint(-1))
        {
            glDeleteTextures(1, &(skyData.sunShaftTexture));
        }

        if(skyData.fboId != GLuint(-1))
        {
            glDeleteFramebuffers(1, &(skyData.fboId));
        }

        SCE::ShaderUtils::DeleteShaderProgram(skyData.skyProgram);
        SCE::ShaderUtils::DeleteShaderProgram(skyData.sunShaftProgram);
    }

}

}
