/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.cpp*********/
/**************************************/

#include "../headers/SCELighting.hpp"
#include "../headers/SCETools.hpp"

using namespace SCE;

#define LIGHT_SHADER_NAME "DeferredLighting"
#define STENCYL_SHADER_NAME "EmptyShader"

SCELighting* SCELighting::s_instance = nullptr;

SCELighting::SCELighting()
    : mDefaultLightShader(-1),
      mLightStencilShader(-1),
      mTexSamplerNames(),
      mTexSamplerUniforms()
{
    //the names ofthe tex sampler uniforms as they appear
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION] = "PositionTex";
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE] = "DiffuseTex";
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL] = "NormalTex";
}


void SCELighting::StartLightRenderPass()
{
    glUseProgram(s_instance->mDefaultLightShader);
}

void SCELighting::StartLightStencilPass()
{
    glUseProgram(s_instance->mLightStencilShader);
}

void SCELighting::initLightShader()
{
    if(mDefaultLightShader == (GLuint) -1){
        mDefaultLightShader = ShaderTools::CompileShader(LIGHT_SHADER_NAME);
    }

    if(mLightStencilShader == (GLuint) -1){
        mLightStencilShader = ShaderTools::CompileShader(STENCYL_SHADER_NAME);
    }

    glUseProgram(mDefaultLightShader);

    for (unsigned int i = 0; i < SCE_GBuffer::GBUFFER_NUM_TEXTURES; i++) {
        mTexSamplerUniforms[i] = glGetUniformLocation(mDefaultLightShader, mTexSamplerNames[i].c_str());
    }
}


void SCELighting::Init()
{
    s_instance = new SCELighting();
    s_instance->initLightShader();
}

void SCELighting::CleanUp()
{
    //unload shader
    glDeleteProgram(s_instance->mDefaultLightShader);
    glDeleteProgram(s_instance->mLightStencilShader);
    delete s_instance;
}

GLuint SCELighting::GetLightShader()
{
    return s_instance->mDefaultLightShader;
}

GLuint SCELighting::GetStencilShader()
{
    return s_instance->mLightStencilShader;
}

GLuint SCELighting::GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType)
{
    return s_instance->mTexSamplerUniforms[textureType];
}
