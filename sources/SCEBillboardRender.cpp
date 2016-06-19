/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*****FILE:SCEBillboardRender.cpp******/
/**************************************/

#include "../headers/SCEBillboardRender.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCEPostProcess.hpp"
#include <glm/gtc/matrix_transform.hpp>


namespace SCE
{
namespace BillboardRender
{

#define BILLBOARD_INTERNAL_FORMAT GL_RGBA16
#define GENERATE_MIPMAPS 1

glm::vec3 GenerateTexturesFromMesh(ui16 nbAngles, ui16 texSize, float borderRatio,
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
        glTexImage2D(GL_TEXTURE_2D, 0, BILLBOARD_INTERNAL_FORMAT,
                     fullSize, fullSize, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
    }    

    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, fullSize, fullSize, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        Debug::RaiseError("FrameBuffer creation error, status:" + std::to_string(status));
    }

    //set viewport
    GLint viewportDims[4];
    glGetIntegerv( GL_VIEWPORT, viewportDims );


    float eyeDist = glm::max(dimensions.x, dimensions.z);
    float biggestXZDim = eyeDist*(1.0f + borderRatio);
    float biggestYDim = (1.0f + borderRatio)*dimensions.y;

    //we use an orthographic projection for simplicity
    glm::mat4 projectionMatrix = glm::ortho(center.x - biggestXZDim, center.x + biggestXZDim,
                                            center.y - biggestYDim, center.y + biggestYDim,
                                            center.z - biggestXZDim, center.z + biggestXZDim);


    projectionMatrix = SCE::Render::FixOpenGLProjectionMatrix(projectionMatrix);

    //render all angles (both normal and difusse)

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::mat4 rotationMatrix;
    glm::mat4 modelMatrix;  

    //set the render targets
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_BLEND);
//    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SCE::Render::ResetClearColorToDefault();

    for(int x = 0; x < root; ++x)
    {
        for(int y = 0; y < root; ++y)
        {
            glViewport(x*texSize, y*texSize, texSize, texSize);

            float angle = float(x*root+y)/float(nbAngles) * 2.0f * glm::pi<float>();
            //compute camera matrix for this angle
            rotationMatrix = glm::rotate(mat4(), -angle, up);
            //kick off render
            renderCallback(modelMatrix, rotationMatrix, projectionMatrix);
        }
    }
    glEnable(GL_CULL_FACE);
//    glEnable(GL_DEPTH_TEST);
//    glDisable(GL_BLEND);

    //unset viewport
    glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);

    // restore default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteTextures(1, &depthTex);

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

    for(int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
#if GENERATE_MIPMAPS
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    return glm::vec3(biggestXZDim, biggestYDim, biggestXZDim);
}


glm::vec4 GetMappingForAgnle(float angleInRad, ui16 nbAngles, bool flipX, float borderRatio)
{
    ui16 root = (ui16)glm::sqrt(nbAngles);
    nbAngles = root*root;

    float PI2 = 2.0f*glm::pi<float>();
    angleInRad = glm::mod(angleInRad + PI2, PI2);
    ui16 prevAng = ui16(angleInRad/PI2*float(nbAngles));

    ui16 xInd = prevAng/root;
    ui16 yInd = prevAng%root;

    float fRoot = (float)root;
    float scaledHalfBorder = borderRatio/fRoot * 0.5f;
    float width = 1.0f/fRoot - scaledHalfBorder;
    float height = 1.0f/fRoot - scaledHalfBorder;

    float xStart = (float)xInd/fRoot + scaledHalfBorder;
    float yStart = (float)yInd/fRoot + scaledHalfBorder;

    if(flipX)
    {
        return vec4(xStart + width, yStart, -width, height);
    }
    else
    {
        return vec4(xStart, yStart, width, height);
    }
}

vec4 GetMappingForAgnle(float angleRad, ui16 nbAngles, float borderRatio)
{
    return GetMappingForAgnle(angleRad, nbAngles, false, borderRatio);
}

vec4 GetMappingForAgnle(float angleRad, ui16 nbAngles, bool flipX)
{
    return GetMappingForAgnle(angleRad, nbAngles, flipX, 0.0f);
}

vec4 GetMappingForAgnle(float angleRad, ui16 nbAngles)
{
    return GetMappingForAgnle(angleRad, nbAngles, false, 0.0f);
}

}
}
