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


#define SUN_TEXTURE_QUALITY 0.2f

namespace SCE
{

namespace SkyRenderer
{

    namespace
    {
        struct SunShaderData
        {
            SunShaderData()
                : sunShaftTexture(-1), sunShaftProgram(-1),
                  sunColor(1.0, 1.0, 0.7)
            {}

            GLuint  sunShaftTexture;
            GLuint  sunShaftProgram;
            GLint   qualityUniform;
            GLint   sunPositionUniform;
            GLint   sunColorUniform;
            vec3    sunColor;
        };

        struct SkyShaderData
        {
            SkyShaderData()
                : skyProgram(-1), skyFadeFactor(7.0f),
                  skyTopColor(0.06, 0.4, 0.85), skyBottomColor(0.65, 0.9, 1.0), fogColor(0.4, 0.7, 0.9)
            {}

            GLuint  skyProgram;
            GLint   sunPositionUniform;
            GLint   skyTopColorUniform;
            GLint   skyBottomColorUniform;
            GLint   skyFadeUniform;
            GLint   sunColorUniform;
            GLint   fogColorUniform;
            float   skyFadeFactor;
            vec3    skyTopColor;
            vec3    skyBottomColor;
            vec3    fogColor;
        };

        struct SkyAndSunData
        {
            SkyAndSunData()
                : fboId(-1)  {}

            //common data
            GLuint  fboId;
            uint    renderWidth;
            uint    renderHeight;
        };

        SkyAndSunData commonSkyData;
        SkyShaderData skyData;
        SunShaderData sunData;
    }


    void Init(uint windowWidth, uint windowHeight)
    {
        float quality = SUN_TEXTURE_QUALITY;

        commonSkyData.renderWidth = float(windowWidth) * quality;
        commonSkyData.renderHeight = float(windowHeight) * quality;

        // Create the FBO
        glGenFramebuffers(1, &(commonSkyData.fboId));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, commonSkyData.fboId);

        glGenTextures(1, &(sunData.sunShaftTexture));
        //texture will store additive sun color as RGB and fog strength as A
        glBindTexture(GL_TEXTURE_2D, sunData.sunShaftTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, commonSkyData.renderWidth, commonSkyData.renderHeight,
                     0, GL_RG, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, sunData.sunShaftTexture, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::RaiseError("FrameBuffer creation error, status:" + std::to_string(status));
        }

        // restore default FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        skyData.skyProgram = SCE::ShaderUtils::CreateShaderProgram("SkyShader");
        sunData.sunShaftProgram = SCE::ShaderUtils::CreateShaderProgram("SunShader");

        sunData.qualityUniform = glGetUniformLocation(sunData.sunShaftProgram, "SizeQuality");
        sunData.sunPositionUniform = glGetUniformLocation(sunData.sunShaftProgram, "SunPosition_worldspace");
        sunData.sunColorUniform = glGetUniformLocation(sunData.sunShaftProgram, "SunColor");

        skyData.sunPositionUniform = glGetUniformLocation(skyData.skyProgram, "SunPosition_worldspace");
        skyData.sunColorUniform = glGetUniformLocation(skyData.skyProgram, "SunColor");
        skyData.skyBottomColorUniform = glGetUniformLocation(skyData.skyProgram, "SkyBottomColor");
        skyData.skyTopColorUniform = glGetUniformLocation(skyData.skyProgram, "SkyTopColor");
        skyData.skyFadeUniform = glGetUniformLocation(skyData.skyProgram, "SkyFadeFactor");
        skyData.fogColorUniform = glGetUniformLocation(skyData.skyProgram, "FogColor");
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
        glViewport(0, 0, commonSkyData.renderWidth, commonSkyData.renderHeight);
        glUseProgram(sunData.sunShaftProgram);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, commonSkyData.fboId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);
        gBuffer.BindTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION,
                            0, 0);
        glUniform1f(sunData.qualityUniform, SUN_TEXTURE_QUALITY);
        glUniform3f(sunData.sunPositionUniform, sunPosition.x, sunPosition.y, sunPosition.z);
        glUniform3fv(sunData.sunColorUniform, 1, &sunData.sunColor[0]);

        //Render sun pass to texture
        SCERender::RenderFullScreenPass(sunData.sunShaftProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);
        //restore viewport
        glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);

        //Render sky
        glUseProgram(skyData.skyProgram);

        gBuffer.BindForSkyPass();
        //bind sun texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, sunData.sunShaftTexture);
        glUniform1i(2, 2);

        glUniform1f(skyData.skyFadeUniform, skyData.skyFadeFactor);
        glUniform3fv(skyData.sunPositionUniform, 1, &sunPosition[0]);
        glUniform3fv(skyData.sunColorUniform, 1, &sunData.sunColor[0]);
        glUniform3fv(skyData.skyBottomColorUniform, 1, &skyData.skyBottomColor[0]);
        glUniform3fv(skyData.skyTopColorUniform, 1, &skyData.skyTopColor[0]);
        glUniform3fv(skyData.fogColorUniform, 1, &skyData.fogColor[0]);

        SCERender::RenderFullScreenPass(skyData.skyProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);

        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    void Cleanup()
    {
        if(sunData.sunShaftTexture != GLuint(-1))
        {
            glDeleteTextures(1, &(sunData.sunShaftTexture));
        }

        if(commonSkyData.fboId != GLuint(-1))
        {
            glDeleteFramebuffers(1, &(commonSkyData.fboId));
        }

        SCE::ShaderUtils::DeleteShaderProgram(skyData.skyProgram);
        SCE::ShaderUtils::DeleteShaderProgram(sunData.sunShaftProgram);
    }

}

}
