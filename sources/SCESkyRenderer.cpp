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
#include "../headers/SCEQuality.hpp"
#include "../headers/SCEPostProcess.hpp"

namespace SCE
{

namespace SkyRenderer
{

#define SUN_SHAFT_TEXTURE_FORMAT GL_R16F

    namespace
    {
        struct SunShaderData
        {
            SunShaderData()
                : sunAndFlareTexture(GL_INVALID_INDEX),
                  sunShaftTexture(GL_INVALID_INDEX),
                  sunFlareProgram(GL_INVALID_INDEX),
                  sunShaftProgram(GL_INVALID_INDEX)
            {}

            GLuint  sunAndFlareTexture;
            GLuint  sunShaftTexture;
            GLuint  sunFlareProgram;
            GLuint  sunShaftProgram;
            GLint   qualityUniform;
            GLint   sunPositionUniform;
        };

        struct SkyShaderData
        {
            SkyShaderData()
                : skyProgram(GL_INVALID_INDEX), skyFadeFactor(7.0f),
                  skyTopColor(0.06, 0.4, 0.85), skyBottomColor(0.65, 0.9, 1.0), fogColor(0.3, 0.6, 0.9)
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
                : fboId(GL_INVALID_INDEX)  {}

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
        float quality = SCE::Quality::SunTextureQuality;

        commonSkyData.renderWidth = uint(float(windowWidth) * quality);
        commonSkyData.renderHeight = uint(float(windowHeight) * quality);

        // Create the FBO
        glGenFramebuffers(1, &(commonSkyData.fboId));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, commonSkyData.fboId);


        glGenTextures(1, &(sunData.sunAndFlareTexture));
        //texture will store additive sun color as RGB and fog strength as A
        glBindTexture(GL_TEXTURE_2D, sunData.sunAndFlareTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, commonSkyData.renderWidth, commonSkyData.renderHeight,
                     0, GL_RG, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, sunData.sunAndFlareTexture, 0);


        glGenTextures(1, &(sunData.sunShaftTexture));
        //texture will store additive sun color as RGB and fog strength as A
        glBindTexture(GL_TEXTURE_2D, sunData.sunShaftTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, SUN_SHAFT_TEXTURE_FORMAT,
                     commonSkyData.renderWidth, commonSkyData.renderHeight,
                     0, GL_RED, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                               GL_TEXTURE_2D, sunData.sunShaftTexture, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::RaiseError("FrameBuffer creation error, status:" + std::to_string(status));
        }

        // restore default FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        skyData.skyProgram = SCE::ShaderUtils::CreateShaderProgram("SkyShader");
        sunData.sunFlareProgram = SCE::ShaderUtils::CreateShaderProgram("SunShader");
        sunData.sunShaftProgram = SCE::ShaderUtils::CreateShaderProgram("LightShafts");

        sunData.qualityUniform = glGetUniformLocation(sunData.sunFlareProgram, "SizeQuality");
        sunData.sunPositionUniform = glGetUniformLocation(sunData.sunFlareProgram, "SunPosition_worldspace");

        skyData.sunPositionUniform = glGetUniformLocation(skyData.skyProgram, "SunPosition_worldspace");
        skyData.sunColorUniform = glGetUniformLocation(skyData.skyProgram, "SunColor");
        skyData.skyBottomColorUniform = glGetUniformLocation(skyData.skyProgram, "SkyBottomColor");
        skyData.skyTopColorUniform = glGetUniformLocation(skyData.skyProgram, "SkyTopColor");
        skyData.skyFadeUniform = glGetUniformLocation(skyData.skyProgram, "SkyFadeFactor");
        skyData.fogColorUniform = glGetUniformLocation(skyData.skyProgram, "FogColor");
    }

    void Render(const SCE::CameraRenderData& renderData, SCE_GBuffer& gBuffer,
                const glm::vec3& sunPosition, const vec4 &sunColor)
    {
        GLint viewportDims[4];
        glGetIntegerv( GL_VIEWPORT, viewportDims );

        //only a screen space quad, don't need depth testing
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);

        //Renders sun and flares
        glViewport(0, 0, commonSkyData.renderWidth, commonSkyData.renderHeight);
        glUseProgram(sunData.sunFlareProgram);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, commonSkyData.fboId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);
        gBuffer.BindTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION, 0, 0);
        glUniform1f(sunData.qualityUniform, SCE::Quality::SunTextureQuality);
        glUniform3f(sunData.sunPositionUniform, sunPosition.x, sunPosition.y, sunPosition.z);

        //Render sun flare pass to texture
        SCE::Render::RenderFullScreenPass(sunData.sunFlareProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);

        //render sun shafts
        glUseProgram(sunData.sunShaftProgram);

        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClear(GL_COLOR_BUFFER_BIT);
        gBuffer.BindTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION, 0, 0);
        glUniform1f(sunData.qualityUniform, SCE::Quality::SunTextureQuality);
        glUniform3f(sunData.sunPositionUniform, sunPosition.x, sunPosition.y, sunPosition.z);

        //Render sun shaft pass to texture
        SCE::Render::RenderFullScreenPass(sunData.sunShaftProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);
        //restore viewport
        glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);

        //blur sun shaft texture
        SCE::PostProcess::BlurTexture2D(sunData.sunShaftTexture,
                                        ivec4(0, 0, commonSkyData.renderWidth, commonSkyData.renderHeight),
                                        8*SCE::Quality::SunTextureQuality, 1, SUN_SHAFT_TEXTURE_FORMAT, GL_RGB);

        //Render sky
        glUseProgram(skyData.skyProgram);
        //binds GBuffer and appropriate textures
        gBuffer.BindForSkyPass();
        //bind sun flare and sun shaft textures
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, sunData.sunAndFlareTexture);
        glUniform1i(2, 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, sunData.sunShaftTexture);
        glUniform1i(3, 3);

        glUniform1f(skyData.skyFadeUniform, skyData.skyFadeFactor);
        glUniform3fv(skyData.sunPositionUniform, 1, &sunPosition[0]);
        glUniform4fv(skyData.sunColorUniform, 1, &sunColor[0]);
        glUniform3fv(skyData.skyBottomColorUniform, 1, &skyData.skyBottomColor[0]);
        glUniform3fv(skyData.skyTopColorUniform, 1, &skyData.skyTopColor[0]);
        glUniform3fv(skyData.fogColorUniform, 1, &skyData.fogColor[0]);

        SCE::Render::RenderFullScreenPass(skyData.skyProgram, renderData.projectionMatrix,
                                        renderData.viewMatrix);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    void Cleanup()
    {
        if(sunData.sunShaftTexture != GL_INVALID_INDEX)
        {
            glDeleteTextures(1, &(sunData.sunShaftTexture));
        }

        if(commonSkyData.fboId != GL_INVALID_INDEX)
        {
            glDeleteFramebuffers(1, &(commonSkyData.fboId));
        }

        SCE::ShaderUtils::DeleteShaderProgram(skyData.skyProgram);
        SCE::ShaderUtils::DeleteShaderProgram(sunData.sunFlareProgram);
    }

    void SetSkyColors(const vec3 &skyBottomColor, const vec3 &skyTopColor, const vec3 &fogColor)
    {
        skyData.fogColor = fogColor;
        skyData.skyBottomColor = skyBottomColor;
        skyData.skyTopColor = skyTopColor;
    }

}

}
