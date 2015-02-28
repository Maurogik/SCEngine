/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCE_GBuffer.cpp*********/
/**************************************/

#include "../headers/SCE_GBuffer.hpp"
#include "../headers/SCETools.hpp"

using namespace SCE;
using namespace std;


SCE_GBuffer::SCE_GBuffer()
    : mFBOId(-1), mDepthTexture(-1)
{

}

SCE_GBuffer::~SCE_GBuffer()
{
    if (mFBOId != 0) {
        glDeleteFramebuffers(1, &mFBOId);
    }

    if (mTextures[0] != 0) {
        glDeleteTextures(GBUFFER_TEXTURE_COUNT, mTextures);
    }

    if (mDepthTexture != 0) {
        glDeleteTextures(1, &mDepthTexture);
    }
}

bool SCE_GBuffer::Init(unsigned int windowWidth, unsigned int windowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &mFBOId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBOId);

    // Create the gbuffer textures
    glGenTextures(GBUFFER_TEXTURE_COUNT, mTextures);
    glGenTextures(1, &mDepthTexture);

    GLenum drawBuffers[GBUFFER_TEXTURE_COUNT];// = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

    for (unsigned int i = 0 ; i < GBUFFER_TEXTURE_COUNT ; i++) {
       glBindTexture(GL_TEXTURE_2D, mTextures[i]);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
       glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
       glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
       //set this texure as frameBufferObject attachment i
       glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mTextures[i], 0);
       drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    // depth
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);

    //set the attachment with the drawBuffers array
    glDrawBuffers(GBUFFER_TEXTURE_COUNT, drawBuffers);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Debug::RaiseError("FrameBuffer creation error, status:" + to_string(status));
        return false;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}

void SCE_GBuffer::BindForWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBOId);
}

char* names[] = {"PositionTex", "DiffuseTex", "NormalTex"};
void SCE_GBuffer::BindForReading()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBOId);
}

void SCE_GBuffer::BindTexturesForShader(GLuint shaderID)
{
    for (unsigned int i = 0 ; i < GBUFFER_TEXTURE_COUNT; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        GLuint loc = glGetUniformLocation(shaderID, names[i]);
        // Set the sampler uniform to the texture unit
        glUniform1i(loc, i);
    }
}

void SCE_GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType)
{
    glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
}
