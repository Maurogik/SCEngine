/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.cpp*********/
/**************************************/

#include "../headers/SCELighting.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/Light.hpp"

using namespace SCE;
using namespace std;

#define LIGHT_SHADER_NAME "DeferredLighting"
#define STENCYL_SHADER_NAME "EmptyShader"

SCELighting* SCELighting::s_instance = nullptr;

SCELighting::SCELighting()
    : mLightShader(-1),
      mStencilShader(-1),
      mTexSamplerNames(),
      mTexSamplerUniforms(),
      mStenciledLights(),
      mDirectionalLights()
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

void SCELighting::RenderLightsToGBuffer(const SCEHandle<Camera> &camera,
                                       SCE_GBuffer& gBuffer)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");   

    //Render with stencil test and no writting to depth buffer
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);

    //render lights needing a stencil pass (Point and Spot lights)
    for(SCEHandle<Light> light : s_instance->mStenciledLights)
    {
        //use (almost) empty shader for stencil pass
        glUseProgram(s_instance->mStencilShader);
        gBuffer.BindForStencilPass();
        s_instance->renderLightStencilPass(camera, light);

        glUseProgram(s_instance->mLightShader);
        gBuffer.BindForLightPass();
        gBuffer.BindTexturesToLightShader();
        s_instance->renderLightingPass(camera, light);
    }

    //Disable stencil because directional lights don't need it
    glDisable(GL_STENCIL_TEST);

    //render directionnal lights
    for(SCEHandle<Light> light : s_instance->mDirectionalLights)
    {
        glUseProgram(s_instance->mLightShader);
        gBuffer.BindForLightPass();
        gBuffer.BindTexturesToLightShader();
        s_instance->renderLightingPass(camera, light);
    }

    //reset depth writting to default
    glDepthMask(GL_TRUE);
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

void SCELighting::RegisterLight(SCEHandle<Light> light)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    s_instance->registerLight(light);
}

void SCELighting::UnregisterLight(SCEHandle<Light> light)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    s_instance->unregisterLight(light);
}

void SCELighting::initLightShader()
{
    if(mLightShader == (GLuint) -1)
    {
        mLightShader = ShaderTools::CompileShader(LIGHT_SHADER_NAME);
    }

    if(mStencilShader == (GLuint) -1)
    {
        mStencilShader = ShaderTools::CompileShader(STENCYL_SHADER_NAME);
    }

    glUseProgram(mLightShader);

    for (unsigned int i = 0; i < SCE_GBuffer::GBUFFER_NUM_TEXTURES; i++)
    {
        mTexSamplerUniforms[i] = glGetUniformLocation(mLightShader, mTexSamplerNames[i].c_str());
    }
}

void SCELighting::registerLight(SCEHandle<Light> light)
{
    vector<SCEHandle<Light>> *targetVector = nullptr;

    switch(light->GetLightType())
    {
    case LightType::DIRECTIONAL_LIGHT :
        targetVector = &mDirectionalLights;
        break;
    case LightType::SPOT_LIGHT :
    case LightType::POINT_LIGHT :
        targetVector = &mStenciledLights;
        break;
    default :
        Debug::RaiseError("Unknown light type found !!");
        break;
    }

    Debug::Assert(find(begin(*targetVector),
                       end(*targetVector),
                       light)
                  == end(*targetVector),
                  "This light is already registered ! That's a problem !");
    targetVector->push_back(light);
}

void SCELighting::unregisterLight(SCEHandle<Light> light)
{
    vector<SCEHandle<Light>>::iterator lightIt;
    vector<SCEHandle<Light>> *targetVector = nullptr;

    switch(light->GetLightType())
    {
    case LightType::DIRECTIONAL_LIGHT :
        targetVector = &mDirectionalLights;
        break;
    case LightType::SPOT_LIGHT :
    case LightType::POINT_LIGHT :
        targetVector = &mStenciledLights;
        break;
    default :
        Debug::RaiseError("Unknown light type found !!");
        break;
    }

    lightIt = find(begin(*targetVector), end(*targetVector), light);
    Debug::Assert(lightIt != end(*targetVector),
                  "This light wasn't found ! That's a problem !");


    targetVector->erase(lightIt);
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
    //setup blending between lighting results
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glStencilMask(0x00); //dont write to stencil buffer in this pass
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);//only render pixels with stendil value > 0

    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    //render light
    light->RenderDeffered(camera);

    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}
