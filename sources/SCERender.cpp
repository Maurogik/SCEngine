/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCERender.cpp**********/
/**************************************/

#include "../headers/SCERender.hpp"
#include "../headers/SCELighting.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCEMeshRender.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCETerrain.hpp"


using namespace SCE;
using namespace std;


#define TONEMAP_EXPOSURE_NAME "SCE_Exposure"
#define TONEMAP_MAX_BRIGHTNESS_NAME "SCE_MaxBrightness"


SCERender* SCERender::s_instance = nullptr;

SCERender::SCERender()
    : mGBuffer(),
      mDefaultClearColor(0.0f, 0.0f, 0.0f, 1.0f),
      mQuadMeshId(-1)
{
    glClearColor(mDefaultClearColor.r,
                 mDefaultClearColor.g,
                 mDefaultClearColor.b,
                 mDefaultClearColor.a);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LEQUAL);
    // Cull triangles which normal is not towards the camera
    glFrontFace(GL_CCW); //this is the default open gl winding order
    glEnable(GL_CULL_FACE);

    //initialize the Gbuffer used to deferred lighting
    mGBuffer.Init(SCECore::GetWindowWidth(), SCECore::GetWindowHeight());

    mQuadMeshId = SCEMeshLoader::CreateQuadMesh();

    mToneMapData.toneMapShader = SCEShaders::CreateShaderProgram("ToneMapping");
    mToneMapData.luminanceShader = SCEShaders::CreateShaderProgram("LuminanceShader");
    mToneMapData.exposureUniform = glGetUniformLocation(mToneMapData.toneMapShader,
                                                        TONEMAP_EXPOSURE_NAME);
    mToneMapData.maxBrightnessUniform = glGetUniformLocation(mToneMapData.toneMapShader,
                                                             TONEMAP_MAX_BRIGHTNESS_NAME);
}

void SCERender::Init()
{
    SCEShaders::Init();
    SCELighting::Init();
    SCEMeshRender::Init();
    SCE::Terrain::Init();
    Debug::Assert(!s_instance, "An instance of the Render system already exists");
    s_instance = new SCERender();
}

void SCERender::CleanUp()
{
    Debug::Assert(s_instance, "No Render system instance found, Init the system before using it");
    delete s_instance;

    SCE::Terrain::Cleanup();
    SCEMeshRender::CleanUp();
    SCELighting::CleanUp();
    SCEShaders::CleanUp();
}

void SCERender::Render(const SCEHandle<Camera>& camera,
                       vector<Container*> objectsToRender)
{
    Debug::Assert(s_instance, "No Render system instance found, Init the system before using it");

    //extract data used for rendering
    CameraRenderData renderData = camera->GetRenderData();

    // Clear the screen (default framebuffer)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render shadows to shadowmap
    SCEHandle<Transform> camTransform = camera->GetContainer()->GetComponent<Transform>();
    glm::mat4 camToWorld = camTransform->GetWorldTransform();
    SCELighting::RenderCascadedShadowMap(renderData, camera->GetFrustrumData(),
                                        camToWorld, objectsToRender);

    //render objects without lighting
    s_instance->renderGeometryPass(camTransform->GetWorldPosition(), renderData, objectsToRender);

    //lighting & sky
    s_instance->mGBuffer.ClearFinalBuffer();
    SCELighting::RenderLightsToGBuffer(renderData, s_instance->mGBuffer);
    SCELighting::RenderSkyToGBuffer(renderData, s_instance->mGBuffer);

    //luminance
    ToneMappingData& tonemap = s_instance->mToneMapData;
    glDisable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    glUseProgram(tonemap.luminanceShader);
    SCEShaders::BindDefaultUniforms(tonemap.luminanceShader);
    s_instance->mGBuffer.BindForLuminancePass();
    RenderFullScreenPass(renderData.projectionMatrix, renderData.viewMatrix);
    s_instance->mGBuffer.GenerateLuminanceMimap();

    //Tonemapping & render to back buffer
    glUseProgram(tonemap.toneMapShader);
    s_instance->mGBuffer.BindForToneMapPass();
    SCEShaders::BindDefaultUniforms(tonemap.toneMapShader);
    glUniform1f(tonemap.exposureUniform, tonemap.exposure);
    glUniform1f(tonemap.maxBrightnessUniform, tonemap.maxBrightness);
    RenderFullScreenPass(renderData.projectionMatrix, renderData.viewMatrix);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    //reset to default framebufffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SCERender::renderGeometryPass(const glm::vec3& cameraPostion_worldspace,
                                   const CameraRenderData& renderData,
                                   std::vector<Container*> objectsToRender)
{
    mGBuffer.BindForGeometryPass();
    // The geometry pass updates the depth buffer
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    SCE::Terrain::RenderTerrain(cameraPostion_worldspace, renderData.projectionMatrix, renderData.viewMatrix, -1.5f);

    for(Container* container : objectsToRender)
    {
        SCEHandle<Material> mat = container->GetComponent<Material>();
        // Use the shader
        mat->BindMaterialData();

        SCEHandle<MeshRenderer> renderer = container->GetComponent<MeshRenderer>();
        renderer->Render(renderData);
    }
}

void SCERender::ResetClearColorToDefault()
{
    Debug::Assert(s_instance, "No Render system instance found, Init the system before using it");
    glClearColor(s_instance->mDefaultClearColor.r,
                 s_instance->mDefaultClearColor.g,
                 s_instance->mDefaultClearColor.b,
                 s_instance->mDefaultClearColor.a);
}

mat4 SCERender::FixOpenGLProjectionMatrix(const mat4& projMat)
{
    //Needed because opengl/glm camera renders along the negative Z axis and
    //I want it to render along the positive axis
    return glm::scale(projMat, glm::vec3(1, 1, -1));
}

void SCERender::RenderFullScreenPass(mat4& projectionMatrix, mat4& viewMatrix)
{
    glm::mat4 modelMatrix = inverse(projectionMatrix * viewMatrix);
    SCEMeshRender::RenderMesh(s_instance->mQuadMeshId, projectionMatrix, viewMatrix, modelMatrix);
}




