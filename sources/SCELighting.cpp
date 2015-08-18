/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.cpp*********/
/**************************************/

#include "../headers/SCELighting.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/Light.hpp"

using namespace SCE;

#define LIGHT_SHADER_NAME "DeferredLighting"
#define STENCYL_SHADER_NAME "EmptyShader"

SCELighting* SCELighting::s_instance = nullptr;

SCELighting::SCELighting()
    : mLightShader(-1),
      mStencilShader(-1),
      mTexSamplerNames(),
      mTexSamplerUniforms()
{
    //the names ofthe tex sampler uniforms as they appear
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION] = "PositionTex";
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE] = "DiffuseTex";
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL] = "NormalTex";
}

void SCELighting::Init()
{
    Debug::Assert(!s_instance, "An instance of the Lighting system already exists");
    s_instance = new SCELighting();
    s_instance->initLightShader();
}

void SCELighting::CleanUp()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    //unload shader
    glDeleteProgram(s_instance->mLightShader);
    glDeleteProgram(s_instance->mStencilShader);
    delete s_instance;
}

void SCELighting::StartLightPass()
{
    //Render lights with stencil test and no writting to depth buffer
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);    
}

void SCELighting::EndLightPass()
{
    //reset to default values
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
}

void SCELighting::RenderLightToGBuffer(const SCEHandle<Camera> &camera,
                                       SCEHandle<Light> &light,
                                       SCE_GBuffer& gBuffer)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");

    //use empty shader for stencil pass (only need to do VS for depth test)
    glUseProgram(s_instance->mStencilShader);
    gBuffer.BindForStencilPass();
    s_instance->renderLightStencilPass(camera, light);

    glUseProgram(s_instance->mLightShader);
    gBuffer.BindForLightPass();
    gBuffer.BindTexturesToLightShader();
    s_instance->renderLightingPass(camera, light);
}

GLuint SCELighting::GetLightShader()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mLightShader;
}

GLuint SCELighting::GetStencilShader()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mStencilShader;
}

GLuint SCELighting::GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mTexSamplerUniforms[textureType];
}

void SCELighting::initLightShader()
{
    if(mLightShader == (GLuint) -1){
        mLightShader = ShaderTools::CompileShader(LIGHT_SHADER_NAME);
    }

    if(mStencilShader == (GLuint) -1){
        mStencilShader = ShaderTools::CompileShader(STENCYL_SHADER_NAME);
    }

    glUseProgram(mLightShader);

    for (unsigned int i = 0; i < SCE_GBuffer::GBUFFER_NUM_TEXTURES; i++) {
        mTexSamplerUniforms[i] = glGetUniformLocation(mLightShader, mTexSamplerNames[i].c_str());
    }
}

void SCELighting::renderLightStencilPass(const SCEHandle<Camera>& camera, SCEHandle<Light> &light)
{
    //avoid writting in color buffer in stencyl pass
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilMask(0xFF); //enable writting to stencil
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    // We need the stencil test to be enabled but we want it
    // to always succeed. Only the depth test matters.
    glStencilFunc(GL_ALWAYS, 0, 0xFF);

    //glStencilOpSeparate(GLenum face,  GLenum sfail,  GLenum dpfail,  GLenum dppass)
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    //Render
    light->RenderToStencil(camera);

    //re enable color writting
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void SCELighting::renderLightingPass(const SCEHandle<Camera>& camera, SCEHandle<Light> &light)
{    
    //glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
//    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ZERO);
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glStencilMask(0x00); //dont write to stencil buffer in this pass

    //only render pixels with stendil value > 0
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
//    glStencilFunc(GL_ALWAYS, 0, 0);

    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    //render light
    light->RenderDeffered(camera);

    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}
