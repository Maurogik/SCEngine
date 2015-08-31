/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCE_GBuffer.cpp*********/
/**************************************/

#include "../headers/SCE_GBuffer.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCELighting.hpp"

using namespace SCE;
using namespace std;


SCE_GBuffer::SCE_GBuffer()
    : mFBOId(-1), mDepthTexture(-1), mFinalTexture(-1)
{

}

SCE_GBuffer::~SCE_GBuffer()
{
    if (mTextures[0] >= 0)
    {
        glDeleteTextures(GBUFFER_TEXTURE_COUNT, mTextures);
    }

    if (mDepthTexture >= 0)
    {
        glDeleteTextures(1, &mDepthTexture);
    }

    if (mFinalTexture >= 0)
    {
        glDeleteTextures(1, &mFinalTexture);
    }

    if (mFBOId >= 0)
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

    for (uint i = 0 ; i < GBUFFER_TEXTURE_COUNT ; i++) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        //set this texture as frameBufferObject color attachment i
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mTextures[i], 0);
    }

    // depth and stencil buffer
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, windowWidth, windowHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);

    // final texture, is needed because we need to render the light pass
    // with the stencil test into the Framebuffer where the stencil buffer was filled (this GBuffer)
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    //bind final texture a the next free attachement (num_textures)
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES,
                           GL_TEXTURE_2D, mFinalTexture, 0);

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
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES);
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
    //clear everything but the diffuse color buffer which was filled in the sky pass
    /*vec4 clearColor = vec4(0.0, 0.0, 0.0, 1.0);
    glClearBufferfv(GL_COLOR, GBUFFER_TEXTURE_TYPE_POSITION, &clearColor[0]);
    glClearBufferfv(GL_COLOR, GBUFFER_TEXTURE_TYPE_NORMAL, &clearColor[0]);*/

}

void SCE_GBuffer::BindForStencilPass()
{
    //write stencil to GBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
}

void SCE_GBuffer::BindForLightPass()
{
    //bind FBO for reading and drawing (because the stencil buffer used for test is the one
    // from the draw framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES);
}

void SCE_GBuffer::BindForSkyPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
//    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_DIFFUSE);
//    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextures[GBUFFER_TEXTURE_TYPE_POSITION]);
    //send uniform ? -> not needed because there is only one texture
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES);
}

void SCE_GBuffer::BindForFinalPass()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBOId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    SetReadBuffer(GBUFFER_NUM_TEXTURES);
}

void SCE_GBuffer::BindTexturesToLightShader()
{
    for (uint i = 0; i < GBUFFER_TEXTURE_COUNT; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        // Set the sampler uniform to the texture unit
        glUniform1i(SCELighting::GetTextureSamplerUniform(GBUFFER_TEXTURE_TYPE(i)), i);
    }
}

void SCE_GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType)
{
    glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
}
