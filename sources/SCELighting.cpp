/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.cpp*********/
/**************************************/

#include "../headers/SCELighting.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/Light.hpp"
#include "../headers/Container.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/Transform.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/Camera.hpp"
#include "../headers/SCESkyRenderer.hpp"
#include "../headers/SCETerrain.hpp"

using namespace SCE;
using namespace std;

#define LIGHT_SHADER_NAME "DeferredLighting"
#define STENCYL_SHADER_NAME "EmptyShader"
#define SHADOWMAP_UNIFORM_NAME "ShadowTex"
#define DEPTH_MAT_UNIFORM_NAME "DepthConvertMat"
#define FAR_SPLIT_UNIFORM_NAME "FarSplits_cameraspace"

#define SHADOW_MAP_WIDTH (2048)
#define SHADOW_MAP_HEIGHT (2048)

#define CASCADE_COUNT 4

SCELighting* SCELighting::s_instance = nullptr;

SCELighting::SCELighting()
    : mLightShader(-1),
      mEmptyShader(-1),
      mTexSamplerNames(),
      mTexSamplerUniforms(),
      mShadowSamplerUnifom(-1),
      mShadowDepthMatUnifom(-1),
      mShadowFarSplitUnifom(-1),
      mShadowMapFBO(),
      mMainLight(nullptr),
      mDepthConvertMatrices(CASCADE_COUNT),
      mFarSplit_cameraspace(CASCADE_COUNT),
      mStenciledLights(),
      mDirectionalLights()
{
    //the names ofthe tex sampler uniforms as they appear
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_POSITION] = "PositionTex";
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE] = "DiffuseTex";
    mTexSamplerNames[SCE_GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL_SPEC] = "NormalTex";

    mShadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, CASCADE_COUNT);

    //initialize per-cascade data
    for(int i = 0; i < CASCADE_COUNT; ++i)
    {
        mDepthConvertMatrices.push_back(glm::mat4(1.0));
        mFarSplit_cameraspace.push_back(0.0f);
    }
}

void SCELighting::Init()
{
    Debug::Assert(!s_instance, "An instance of the Lighting system already exists");
    s_instance = new SCELighting();
    s_instance->initLightShader();

    //init sky renderer
    GLint viewportDims[4];
    glGetIntegerv( GL_VIEWPORT, viewportDims );
    SCE::SkyRenderer::Init(viewportDims[2], viewportDims[3]);
}

void SCELighting::CleanUp()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    //unload shader
    SCE::ShaderUtils::DeleteShaderProgram(s_instance->mLightShader);
    SCE::ShaderUtils::DeleteShaderProgram(s_instance->mEmptyShader);
    delete s_instance;

    //clean sky renderer
    SCE::SkyRenderer::Cleanup();
}

void SCELighting::RenderCascadedShadowMap(const CameraRenderData &camRenderData,
                                         FrustrumData camFrustrumData,
                                         glm::mat4 camToWorldMat,
                                         std::vector<Container*> objectsToRender)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");

    if(!s_instance->mMainLight)
    {
        return;
    }

    //Julie ! Do the thing !
    vector<CameraRenderData> splitShadowFrustrums
            = s_instance->computeCascadedLightFrustrums(camFrustrumData,
                                                        camToWorldMat);

    for(uint i = 0; i < splitShadowFrustrums.size(); ++i)
    {
        //render shadowMap from light point of view
        s_instance->renderShadowmapPass(splitShadowFrustrums[i], objectsToRender, i);
    }
}

void SCELighting::RenderLightsToGBuffer(const CameraRenderData& renderData,
                                       SCE_GBuffer& gBuffer)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");   

    glDepthMask(GL_FALSE);

    //Render with stencil test and no writting to depth buffer
    glEnable(GL_STENCIL_TEST);

    //render lights needing a stencil pass (Point and Spot lights)
    for(SCEHandle<Light> light : s_instance->mStenciledLights)
    {
        glUseProgram(s_instance->mEmptyShader);        
        gBuffer.BindForStencilPass();
        s_instance->renderLightStencilPass(renderData, light);

        glUseProgram(s_instance->mLightShader);
        gBuffer.BindForLightPass();
        gBuffer.BindTexturesForLighting();

        //bind for shadow calculations even if there won't be shadows on screen
        s_instance->mShadowMapFBO.BindTextureToLightShader(SCE_GBuffer::GBUFFER_NUM_TEXTURES);
        glUniformMatrix4fv(s_instance->mShadowDepthMatUnifom, CASCADE_COUNT, GL_FALSE,
                           &(s_instance->mDepthConvertMatrices[0][0][0]));
        glUniform1fv(s_instance->mShadowFarSplitUnifom, CASCADE_COUNT,
                     &(s_instance->mFarSplit_cameraspace[0]));

        s_instance->renderLightingPass(renderData, light);
    }

    //Disable stencil because directional lights don't need it
    glDisable(GL_STENCIL_TEST);

    //render directionnal lights
    for(SCEHandle<Light> light : s_instance->mDirectionalLights)
    {
        glUseProgram(s_instance->mLightShader);
        gBuffer.BindForLightPass();
        gBuffer.BindTexturesForLighting();

        //bind the shadowmap to the next free texture unit
        s_instance->mShadowMapFBO.BindTextureToLightShader(SCE_GBuffer::GBUFFER_NUM_TEXTURES);

        //bind the world-to-depth convertion matrices
        glUniformMatrix4fv(s_instance->mShadowDepthMatUnifom, CASCADE_COUNT, GL_FALSE,
                           &(s_instance->mDepthConvertMatrices[0][0][0]));

        //bind the far split planes usd to pick a cascade level
        glUniform1fv(s_instance->mShadowFarSplitUnifom, CASCADE_COUNT,
                     &(s_instance->mFarSplit_cameraspace[0]));

        s_instance->renderLightingPass(renderData, light);
    }


    //reset depth writting to default
    glDepthMask(GL_TRUE);
}

void SCELighting::RenderSkyToGBuffer(const CameraRenderData& renderData, SCE_GBuffer& gBuffer)
{
    vec3 sunPos = s_instance->mMainLight->GetContainer()->GetComponent<Transform>()->GetWorldPosition();
    //Render sky and sun
    SCE::SkyRenderer::Render(renderData, gBuffer, sunPos);
}


GLuint SCELighting::GetLightShader()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mLightShader;
}

//Returns an empty shader, may be something else later ?
GLuint SCELighting::GetStencilShader()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mEmptyShader;
}

//Returns an empty shader, may be something else later ?
GLuint SCELighting::GetShadowMapShader()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mEmptyShader;
}

GLuint SCELighting::GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mTexSamplerUniforms[textureType];
}

GLuint SCELighting::GetShadowmapSamplerUniform()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mShadowSamplerUnifom;
}

void SCELighting::RegisterLight(SCEHandle<Light> light)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    s_instance->registerLight(light);
    light->InitLightRenderData(s_instance->mLightShader);
}

void SCELighting::UnregisterLight(SCEHandle<Light> light)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    s_instance->unregisterLight(light);
}

void SCELighting::SetSunLight(SCEHandle<Light> light)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    Debug::Assert(!s_instance->mMainLight, "A shadow caster has already been set");
    Debug::Assert(light->GetLightType() == DIRECTIONAL_LIGHT,
                  "Shadow casting is only supported for directional lights");

    s_instance->mMainLight = light;
}

void SCELighting::initLightShader()
{
    if(mLightShader == (GLuint) -1)
    {
        mLightShader = SCE::ShaderUtils::CreateShaderProgram(LIGHT_SHADER_NAME);
    }

    if(mEmptyShader == (GLuint) -1)
    {
        mEmptyShader = SCE::ShaderUtils::CreateShaderProgram(STENCYL_SHADER_NAME);
    }

    for (uint i = 0; i < SCE_GBuffer::GBUFFER_NUM_TEXTURES; i++)
    {
        mTexSamplerUniforms[i] = glGetUniformLocation(mLightShader, mTexSamplerNames[i].c_str());
    }
    mShadowDepthMatUnifom = glGetUniformLocation(mLightShader, DEPTH_MAT_UNIFORM_NAME);
    mShadowSamplerUnifom = glGetUniformLocation(mLightShader, SHADOWMAP_UNIFORM_NAME);
    mShadowFarSplitUnifom = glGetUniformLocation(mLightShader, FAR_SPLIT_UNIFORM_NAME);


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

void SCELighting::renderLightStencilPass(const CameraRenderData& renderData, SCEHandle<Light> &light)
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
    light->RenderWithoutLightData(renderData);

    //re enable color writting
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void SCELighting::renderLightingPass(const CameraRenderData& renderData, SCEHandle<Light> &light)
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
    light->RenderWithLightData(renderData);

    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void SCELighting::renderShadowmapPass(const CameraRenderData& lightRenderData,
                                      std::vector<Container*> objectsToRender,
                                      uint shadowmapId)
{
    GLint viewportDims[4];
    glGetIntegerv( GL_VIEWPORT, viewportDims );

    glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

    glUseProgram(mEmptyShader);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);//render only back faces to avoid some shadow acnee

    mShadowMapFBO.BindForShadowPass(shadowmapId);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    for(Container* container : objectsToRender)
    {
        container->GetComponent<MeshRenderer>()->Render(lightRenderData);
    }

//    SCE::Terrain::RenderTerrain(lightRenderData.projectionMatrix, lightRenderData.viewMatrix, 16.0f);

    glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );

    mDepthConvertMatrices[shadowmapId] = biasMatrix * lightRenderData.projectionMatrix
                       * lightRenderData.viewMatrix;

    glViewport(viewportDims[0], viewportDims[1], viewportDims[2], viewportDims[3]);
    glCullFace(GL_BACK);
}

std::vector<CameraRenderData> SCELighting::computeCascadedLightFrustrums(FrustrumData cameraFrustrum,
                                                                         glm::mat4 camToWorldMat)
{
    glm::mat4 lightToWorld = mMainLight->GetContainer()->GetComponent<Transform>()->GetWorldTransform();
    glm::mat4 worldToLight = glm::inverse(lightToWorld);

    vector<CameraRenderData> lightFrustrums;

    float lambda    = 0.75f;//split correction strength
    float near      = cameraFrustrum.near;
    float far       = cameraFrustrum.far;
    float ratio     = far / near;

    //compute where the camera frustrum will be split
    vec2 zSplits[CASCADE_COUNT]; //x is near dist, y is far dist

    uint cascadeCount = CASCADE_COUNT;
    zSplits[0].x = near;

    for(uint i = 1; i < cascadeCount; ++i)
    {
        float si = i / (float)cascadeCount;

        zSplits[i].x = lambda * (near * glm::pow(ratio, si))
                       + (1.0f - lambda) * (near + (far - near) * si);
        zSplits[i - 1].y = zSplits[i].x * 1.005f;//slightly offset to fix holes ?
    }

    zSplits[cascadeCount - 1].y = far;

    for(uint i = 0; i < cascadeCount; ++i)
    {
        //compute bouding box of camera frustrum in light space
        float max       = std::numeric_limits<float>::max() - 1;//just a big float (the biggest !)
        float far       = -max;
        float near      = max;
        float top       = -max;
        float bottom    = max;
        float left      = max;
        float right     = -max;

        float splitCamNearZ  = zSplits[i].x;
        float splitCamFarZ   = zSplits[i].y;

        FrustrumData splitFrustrum = cameraFrustrum;
        splitFrustrum.near = splitCamNearZ;
        splitFrustrum.far = splitCamFarZ;

        //get the points that enclose the splitted frustrum
        vector<vec3> frustrumCorners(Camera::GetFrustrumCorners(splitFrustrum, camToWorldMat));

        for(vec3& corner : frustrumCorners)
        {
            vec4 corner_lightSpace = worldToLight * vec4(corner, 1.0);

            far     = glm::max(far, corner_lightSpace.z);
            near    = glm::min(near, corner_lightSpace.z);
            right   = glm::max(right, corner_lightSpace.x);
            left    = glm::min(left, corner_lightSpace.x);
            top     = glm::max(top, corner_lightSpace.y);
            bottom  = glm::min(bottom, corner_lightSpace.y);
        }

        //near should be close (actually, it should be the closest occluder,
        //but we don't know where it is yet)
        near = 10.0f;

        //create an orthographic projection enclosing all points between the light
        //and the splitted camera frustrum
        CameraRenderData lightRenderData;
        glm::mat4 projMat = glm::ortho(left, right,
                                       bottom, top,
                                       near, far);

        projMat = SCERender::FixOpenGLProjectionMatrix(projMat);
        lightRenderData.viewMatrix = worldToLight;
        lightRenderData.projectionMatrix = projMat;

        lightFrustrums.push_back(lightRenderData);

        //store the far split to check against it in sahder in order to pick cascade
        mFarSplit_cameraspace[i] = splitCamFarZ;
    }

    return lightFrustrums;
}

