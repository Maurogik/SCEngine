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

#define USE_PCF

SCEShadowMap::SCEShadowMap()
    : mFBOId(-1),
      mDepthTexture(-1)
{

}

SCEShadowMap::~SCEShadowMap()
{
    if(mDepthTexture != GLuint(-1))
    {
        glDeleteTextures(1, &mDepthTexture);
    }

    if(mFBOId != GLuint(-1))
    {
        glDeleteFramebuffers(1, &mFBOId);
    }
}

bool SCEShadowMap::Init(GLuint shadowmapWidth, GLuint shadowmapHeight, GLuint cascadeCount)
{
    // Create the FBO
    glGenFramebuffers(1, &mFBOId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBOId);

    glGenTextures(1, &mDepthTexture);
    // depth and stencil buffer
    glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexture);

    //texture array creation
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT16,
                 shadowmapWidth, shadowmapHeight, cascadeCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

#ifdef USE_PCF
    //to allow PCF shadows
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#else
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
#endif

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Debug::RaiseError("FrameBuffer creation error, status:" + to_string(status));
        return false;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}

void SCEShadowMap::BindForShadowPass(GLuint cascadeId)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOId);
    //bind the right level of the texture array
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0, cascadeId);
}

void SCEShadowMap::BindTextureToLightShader(GLuint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexture);
    // Set the sampler uniform to the texture unit
    glUniform1i(SCELighting::GetShadowmapSamplerUniform(), textureUnit);
}


