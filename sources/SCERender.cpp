/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCERender.cpp**********/
/**************************************/

#include "../headers/SCERender.hpp"
#include "../headers/SCELighting.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCETools.hpp"

using namespace SCE;
using namespace std;

SCERender* SCERender::s_instance = nullptr;

SCERender::SCERender()
    : mGBuffer(),
      mDefaultClearColor(0.0f, 0.0f, 0.0f, 1.0f)
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
    mGBuffer.Init(SCECore::GetWindowWidth(), SCECore::GetWindowHeight(),
                  SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
}

void SCERender::Init()
{
    Debug::Assert(!s_instance, "An instance of the Render system already exists");
    s_instance = new SCERender();
}

void SCERender::CleanUp()
{
    Debug::Assert(s_instance, "No Render system instance found, Init the system before using it");
    delete s_instance;
}

void SCERender::Render(const SCEHandle<Camera>& camera,
                       vector<Container*> objectsToRender)
{
    Debug::Assert(s_instance, "No Render system instance found, Init the system before using it");

    GLsizei width = SCECore::GetWindowWidth();
    GLsizei height = SCECore::GetWindowHeight();

    //extract data used for rendering
    SCECameraData renderData = camera->GetRenderData();


    // Clear the screen (default framebuffer)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render objects without lighting
    s_instance->renderGeometryPass(renderData, objectsToRender);

    s_instance->mGBuffer.ClearFinalBuffer();

    SCELighting::RenderShadowsToGBuffer(renderData, camera->GetFrustrumCorners(),
                                        objectsToRender, s_instance->mGBuffer);

    SCELighting::RenderLightsToGBuffer(renderData, s_instance->mGBuffer);

    //Render final image from GBuffer to window framebuffer
    s_instance->mGBuffer.BindForFinalPass();

    //TODO hook post-processing pipeline here
    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    //reset to default framebufffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SCERender::renderGeometryPass(const SCECameraData& renderData,
                                   std::vector<Container*> objectsToRender)
{
    mGBuffer.BindForGeometryPass();
    // The geometry pass updates the depth buffer
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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
    //Needed because opengl camera renders along the negative Z axis and I want it to render along the positiv axis
   return glm::scale(projMat, glm::vec3(1, 1, -1));
}




