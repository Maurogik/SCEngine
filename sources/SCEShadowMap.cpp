/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEShadowMap.cpp*********/
/**************************************/

#include "../headers/SCEShadowMap.hpp"
#include "../headers/SCELighting.hpp"
#include "../headers/SCETools.hpp"

using namespace SCE;
using namespace std;


SCEShadowMap::SCEShadowMap()
    : mFBOId(-1),
      mDepthTexture(-1)
{

}

SCEShadowMap::~SCEShadowMap()
{
    if(mDepthTexture >= 0)
    {
        glDeleteTextures(1, &mDepthTexture);
    }

    if(mFBOId >= 0)
    {
        glDeleteFramebuffers(1, &mFBOId);
    }
}

bool SCEShadowMap::Init(GLuint shadowmapWidth, GLuint shadowmapHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &mFBOId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBOId);

    glGenTextures(1, &mDepthTexture);
    // depth and stencil buffer
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowmapWidth, shadowmapHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Debug::RaiseError("FrameBuffer creation error, status:" + to_string(status));
        return false;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}

void SCEShadowMap::BindForShadowPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
}

void SCEShadowMap::BindTextureToLightShader(GLuint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    // Set the sampler uniform to the texture unit
    glUniform1i(SCELighting::GetShadowmapSamplerUniform(), textureUnit);
}


