/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCE_GBuffer.cpp*********/
/**************************************/

#include "../headers/SCE_GBuffer.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCELighting.hpp"
#include "../headers/SCEPostProcess.hpp"

using namespace SCE;
using namespace std;

#define FINAL_TEXT_ATTACHMENT (GBUFFER_NUM_TEXTURES)
#define LUM_TEXT_ATTACHMENT (GBUFFER_NUM_TEXTURES + 1)

SCE_GBuffer::SCE_GBuffer()
    : mFBOId(GL_INVALID_INDEX),
      mDepthTexture(GL_INVALID_INDEX),
      mFinalTexture(GL_INVALID_INDEX),
      mLuminanceTexture(GL_INVALID_INDEX)
{

}

SCE_GBuffer::~SCE_GBuffer()
{
    if (mTextures[0] != GL_INVALID_INDEX)
    {
        glDeleteTextures(GBUFFER_TEXTURE_COUNT, mTextures);
    }

    if (mDepthTexture != GL_INVALID_INDEX)
    {
        glDeleteTextures(1, &mDepthTexture);
    }

    if (mFinalTexture != GL_INVALID_INDEX)
    {
        glDeleteTextures(1, &mFinalTexture);
    }

    if (mLuminanceTexture != GL_INVALID_INDEX)
    {
        glDeleteTextures(1, &mLuminanceTexture);
    }

    if (mFBOId != GL_INVALID_INDEX)
    {
        glDeleteFramebuffers(1, &mFBOId);
    }
}

bool SCE_GBuffer::Init(uint windowWidth, uint windowHeight)
{

    // Create the FBO
    glGenFramebuffers(1, &mFBOId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBOId);

    // Create the gbuffer textures
    glGenTextures(GBUFFER_TEXTURE_COUNT, mTextures);
    glGenTextures(1, &mDepthTexture);
    glGenTextures(1, &mFinalTexture);
    glGenTextures(1, &mLuminanceTexture);

    for (uint i = 0 ; i < GBUFFER_TEXTURE_COUNT ; i++) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        if(i == GBUFFER_TEXTURE_TYPE_NORMAL_SPEC || i == GBUFFER_TEXTURE_TYPE_DIFFUSE)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        }
        else //use only 3 channels for position
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
        }
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        //set this texture as frameBufferObject color attachment i
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mTextures[i], 0);
    }

    // depth and stencil buffer
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);


    // final texture, is needed because we need to render the light pass
    // with the stencil test into the Framebuffer where the stencil buffer was filled (this GBuffer)
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + FINAL_TEXT_ATTACHMENT,
                           GL_TEXTURE_2D, mFinalTexture, 0);


    glBindTexture(GL_TEXTURE_2D, mLuminanceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + LUM_TEXT_ATTACHMENT,
                           GL_TEXTURE_2D, mLuminanceTexture, 0);


    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Debug::RaiseError("FrameBuffer creation error, status:" + to_string(status));
        return false;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}

void SCE_GBuffer::ClearFinalBuffer()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBOId);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + FINAL_TEXT_ATTACHMENT);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SCE_GBuffer::BindForGeometryPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
    //reset the color attachment buffers that have been removed for stencil pass
    GLenum drawBuffers[GBUFFER_TEXTURE_COUNT];
    for (uint i = 0 ; i < GBUFFER_TEXTURE_COUNT ; i++) {
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    //set the attachment with the drawBuffers array
    glDrawBuffers(GBUFFER_TEXTURE_COUNT, drawBuffers);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void SCE_GBuffer::BindForStencilPass()
{
    //write stencil to GBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
}

void SCE_GBuffer::BindForLightPass()
{
    //bind FBO for reading and drawing (because the stencil buffer used for stencil test is the one
    // from the draw framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + FINAL_TEXT_ATTACHMENT);
}

void SCE_GBuffer::BindForSkyPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glUniform1i(0, 0);//FinalColorTex is sampler0

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextures[GBUFFER_TEXTURE_TYPE_POSITION]);
    glUniform1i(1, 1);//PositionTex is sampler1

    glDrawBuffer(GL_COLOR_ATTACHMENT0 + FINAL_TEXT_ATTACHMENT);
}

void SCE_GBuffer::BindForLuminancePass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glUniform1i(0, 0);//FinalColorTex is sampler0

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mLuminanceTexture);
    glUniform1i(1, 1);//LuminanceTex is sampler1

    glDrawBuffer(GL_COLOR_ATTACHMENT0 + LUM_TEXT_ATTACHMENT);
}

void SCE_GBuffer::GenerateLuminanceMimap()
{
    glBindTexture(GL_TEXTURE_2D, mLuminanceTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void SCE_GBuffer::BindForToneMapPass()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBOId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glUniform1i(0, 0);//FinalColorTex is sampler0

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mLuminanceTexture);
    glUniform1i(1, 1);//LuminanceTex is sampler1
}

void SCE_GBuffer::SetupTexturesForLighting()
{
    for (uint i = 0; i < GBUFFER_TEXTURE_COUNT; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        // Set the sampler uniform to the texture unit
        glUniform1i(SCELighting::GetTextureSamplerUniform(GBUFFER_TEXTURE_TYPE(i)), i);
    }
}

void SCE_GBuffer::SetupFinalTexture(uint uniform, uint sampler)
{
    glActiveTexture(GL_TEXTURE0 + sampler);
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glUniform1i(uniform, sampler);
}

void SCE_GBuffer::BindTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE type, uint uniform, uint texUnit)
{
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, mTextures[type]);
    // Set the sampler uniform to the texture unit
    glUniform1i(uniform, texUnit);
}

void SCE_GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType)
{
    glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
}
