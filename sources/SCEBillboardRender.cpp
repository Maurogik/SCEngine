/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*****FILE:SCEBillboardRender.cpp******/
/**************************************/

#include "../headers/SCEBillboardRender.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCERender.hpp"
#include <glm/gtc/matrix_transform.hpp>


namespace SCE
{
namespace BillboardRender
{

#if 1
void GenerateTexturesFromMesh(ui16 nbAngles, ui16 texSize,
                              glm::vec3 const& center, glm::vec3 const& dimensions,
                              GLuint* diffuseTex, GLuint* normalTex,
                              RenderCallback renderCallback)
{
    ui16 root = ui16(glm::sqrt(nbAngles));
    nbAngles = root*root;
    ui16 fullSize = root*texSize;

    //create FBO
    GLuint fboId;
    GLuint textures[2];
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    //create tex arrays
    glGenTextures(2, textures);    

    for(int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     fullSize, fullSize, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
    }    

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        Debug::RaiseError("FrameBuffer creation error, status:" + std::to_string(status));
        return;
    }

    //set viewport
    GLint viewportDims[4];
    glGetIntegerv( GL_VIEWPORT, viewportDims );


    float biggestDim = glm::max(dimensions.x, glm::max(dimensions.y, dimensions.z));

    //we use an orthographic projection for simplicity
    glm::mat4 projectionMatrix = glm::ortho(center.x - biggestDim, center.x + biggestDim,
                                            center.y - biggestDim, center.y + biggestDim,
                                            center.z - biggestDim, center.z + biggestDim);
    projectionMatrix = SCE::Render::FixOpenGLProjectionMatrix(projectionMatrix);

    //render all angles (both normal and difusse)
    glm::vec3 eyeStart(0.0f, 0.0f, -biggestDim);
    glm::vec3 target(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 eyePos;
    glm::mat4 rotationMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;  

    //set the render targets
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    SCE::Render::ResetClearColorToDefault();

    for(int i = 0; i < root; ++i)
    {
        for(int j = 0; j < root; ++j)
        {
            glViewport(i*texSize, j*texSize, texSize, texSize);

            float angle = float(i*root+j)/float(nbAngles) * 2.0f * glm::pi<float>();
            //compute camera matrix for this angle
            rotationMatrix = glm::rotate(rotationMatrix, angle, up);
            eyePos = vec3(rotationMatrix*vec4(eyeStart, 1.0f));
            viewMatrix = glm::lookAtLH(eyePos, target, up);
            //kick off render
            renderCallback(modelMatrix, viewMatrix, projectionMatrix);
        }
    }

    //unset viewport
    glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);

    // restore default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //delete FBO
    glDeleteFramebuffers(1, &fboId);

    if(diffuseTex)
    {
        *diffuseTex = textures[0];
    }

    if(normalTex)
    {
        *normalTex = textures[1];
    }
}

#else

void GenerateTexturesFromMesh(ui16 nbAngles, ui16 texSize, glm::vec3 dimensions,
                              GLuint* diffuseTex, GLuint* normalTex,
                              RenderCallback renderCallback)
{
    ui16 root = glm::sqrt(nbAngles);
    nbAngles = root*root;
    ui16 fullSize = texSize;

    //create FBO
    GLuint fboId;
    GLuint textures[2];
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    //create tex arrays
    glGenTextures(2, textures);

    for(int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA16F,
                     fullSize, fullSize, nbAngles, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textures[i], 0, 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        Debug::RaiseError("FrameBuffer creation error, status:" + std::to_string(status));
        return;
    }

    //set viewport
    GLint viewportDims[4];
    glGetIntegerv( GL_VIEWPORT, viewportDims );
    glViewport(0, 0, fullSize, fullSize);


    //we use an orthographic projection for simplicity
    float biggestDim = glm::max(glm::max(dimensions.x, dimensions.y), dimensions.z);

    glm::mat4 projectionMatrix = glm::ortho(-biggestDim, biggestDim,
                                            -biggestDim, biggestDim,
                                            -biggestDim, biggestDim);
    projectionMatrix = SCE::Render::FixOpenGLProjectionMatrix(projectionMatrix);

    //render all angles (both normal and difusse)
    glm::vec3 eyeStart(0.0f, 0.0f, -biggestDim);
    glm::vec3 target(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 eyePos;
    glm::mat4 rotationMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;

    //set the render targets
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    SCE::Render::ResetClearColorToDefault();

    for(int i = 0; i < root; ++i)
    {
        for(int j = 0; j < root; ++j)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0, i*root+j);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures[1], 0, i*root+j);

            float angle = float(i*root+j)/float(nbAngles) * 2.0f * M_PI;
            //compute camera matrix for this angle
            rotationMatrix = glm::rotate(rotationMatrix, angle, up);
            eyePos = vec3(rotationMatrix*vec4(eyeStart, 1.0f));
            viewMatrix = glm::lookAtLH(eyePos, target, up);
            //kick off render
            renderCallback(modelMatrix, viewMatrix, projectionMatrix);
        }
    }

    //unset viewport
    glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);

    // restore default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //delete FBO
    glDeleteFramebuffers(1, &fboId);

    if(diffuseTex)
    {
        *diffuseTex = textures[0];
    }

    if(normalTex)
    {
        *normalTex = textures[1];
    }
}

#endif

ui16 GetIdFromAngle(float angleInRad, ui16 nbAngles)
{
    ui16 root = (ui16)glm::sqrt(nbAngles);
    nbAngles = root*root;

    float PI2 = 2.0f*glm::pi<float>();
    angleInRad = glm::mod(angleInRad + PI2, PI2);
    ui16 prevAng = ui16(angleInRad/PI2*float(nbAngles));

    return 1;
    return prevAng;
}

}
}
