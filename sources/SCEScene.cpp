/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Scene.cpp **********/
/**************************************/

#include "../headers/SCEScene.hpp"
#include "../headers/MeshRenderer.hpp"
#include "../headers/Camera.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/GameObject.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCELighting.hpp"

using namespace SCE;
using namespace std;


SCEScene* SCEScene::s_scene = nullptr;

SCE::SCEScene::SCEScene()
    : mContainers(), mLights(), mGameObjects(), mLastId(0), mLightingGBuffer(),
      mDefaultClearColor(0.0f, 0.0f, 0.0f, 1.0f)
{    
    resetClearColorToDefault();
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LEQUAL);
    // Cull triangles which normal is not towards the camera
    glFrontFace(GL_CCW); //this is the default open gl winding order
    glEnable(GL_CULL_FACE);

    //initialize the Gbuffer used to deferred lighting
    mLightingGBuffer.Init(SCECore::GetWindowWidth(), SCECore::GetWindowHeight());
}

SCE::SCEScene::~SCEScene()
{
    Internal::Log("Delete scene");
    Internal::Log("Clear stuff");
    for(auto cont : mContainers)
    {
        delete(cont);
    }
    mContainers.clear();
    mGameObjects.clear();
    mLights.clear();
}

void SCE::SCEScene::CreateEmptyScene()
{
    Debug::Assert(s_scene == nullptr
                  , "A scene is already loaded, destroy it befor loading a new one");
    s_scene = new SCEScene();
}

void SCE::SCEScene::LoadScene(const std::string& scenePath)
{
    Debug::Assert(s_scene == nullptr
                  , "A scene is already loaded, destroy it befor loading a new one");
}


void SCE::SCEScene::Run()
{
    Debug::Assert(s_scene
                  , "There is no scene to display, create or load a scene before running the engine");
    s_scene->UpdateScene();
    s_scene->RenderScene();
}


void SCE::SCEScene::RenderScene()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Parse cameras and render them
    //TODO order cameras by depth first
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<Camera>()){
            SCEHandle<Camera> cam = mContainers[i]->GetComponent<Camera>();
            renderSceneWithCamera(cam);
        }
    }
}

void SCEScene::UpdateScene()
{
    //TODO handle case where an object is waiting to be destroyed

    vector<SCEHandle<GameObject> > tmpGameObjects = s_scene->mGameObjects;

    for(size_t i = 0; i < tmpGameObjects.size(); ++i){
        tmpGameObjects[i]->Update();
    }

    /************************************************************
     * Temporary
     **/
    //SCEInternal::InternalMessage("move control code to appropriate location");

    //move camera

    SCEHandle<Container> contPtr;
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<Camera>()){
            contPtr = SCEHandle<Container>(mContainers[i]);
            break;
        }
    }

    if(!contPtr)
        return;


    GLFWwindow* window = SCECore::GetWindow();
    SCEHandle<Transform> transform = contPtr->GetComponent<Transform>();

    double xMouse = 0.0;
    double yMouse = 0.0;

    glfwGetCursorPos(window, &xMouse, &yMouse);

    static double prevX = 0.0;
    static double prevY = 0.0;

    double xDelta = xMouse - prevX;
    double yDelta = yMouse - prevY;

    prevX = xMouse;
    prevY = yMouse;

    float deltaTime = SCETime::DeltaTime();

    float rotateSpeed = 250.0f;
    float speed = 5.0f;

    vec3 position = transform->GetWorldPosition();
    transform->RotateAroundAxis(vec3(0, 1, 0), xDelta * rotateSpeed * deltaTime);
    transform->RotateAroundAxis(vec3(1, 0, 0), yDelta * rotateSpeed * deltaTime);  

    vec3 forward = transform->Forward();
    vec3 left = transform->Left();

    // Move forward
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        position += forward * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        position += -forward * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        position += -left * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        position += left * deltaTime * speed;
    }

    transform->SetWorldPosition(position);
    /** End of temporary**/
}

void SCE::SCEScene::DestroyScene()
{
    SECURE_DELETE(s_scene);
}

SCEHandle<Container> SCEScene::CreateContainer(const string &name)
{
    if(!s_scene){
        Debug::LogError("No scene to add the container to, create a scene first");
    }
    Container* cont = new Container(name, ++s_scene->mLastId);
    s_scene->mContainers.push_back(cont);
    return SCEHandle<Container>(cont);
}

void SCEScene::DestroyContainer(const SCEHandle<Container> &container)
{
    RemoveContainer(container->GetContainerId());
}


void SCEScene::RemoveContainer(int objId)
{
    Internal::Log("Removing container from scene");
    if(!s_scene) return;
    auto objIt = find_if(
                     begin(s_scene->mContainers)
                     , end(s_scene->mContainers)
                     , [&objId](Container* cont) {
                 return cont->GetContainerId() == objId;
});
    if(objIt != end(s_scene->mContainers)) {
        delete(*objIt);
        s_scene->mContainers.erase(objIt);
    }
}

std::vector<SCEHandle<Container> > SCEScene::FindContainersWithTag(const string &tag)
{
    vector<SCEHandle<Container> > tagged;
    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->GetTag() == tag){
            tagged.push_back(SCEHandle<Container>(s_scene->mContainers[i]));
        }
    }
    return tagged;
}

std::vector<SCEHandle<Container> > SCEScene::FindContainersWithLayer(const string &layer)
{
    vector<SCEHandle<Container> >  layered;
    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->GetLayer() == layer){
            layered.push_back(SCEHandle<Container>(s_scene->mContainers[i]));
        }
    }
    return layered;
}

void SCEScene::RegisterGameObject(SCEHandle<GameObject> gameObject)
{
    if(find(begin(s_scene->mGameObjects), end(s_scene->mGameObjects), gameObject) == end(s_scene->mGameObjects)){
        s_scene-> mGameObjects.push_back(gameObject);
    }
}

void SCEScene::UnregisterGameObject(SCEHandle<GameObject> gameObject)
{
    auto it = find(begin(s_scene->mGameObjects), end(s_scene->mGameObjects), gameObject);
    if(it != end(s_scene->mGameObjects)){
        s_scene-> mGameObjects.erase(it);
    }
}

void SCEScene::RegisterLight(SCEHandle<Light> light)
{
    if(find(begin(s_scene->mLights), end(s_scene->mLights), light) == end(s_scene->mLights)){
        s_scene->mLights.push_back(light);
    }
}

void SCEScene::UnregisterLight(SCEHandle<Light> light)
{
    auto it = find(begin(s_scene->mLights), end(s_scene->mLights), light);
    if(it != end(s_scene->mLights)){
        s_scene->mLights.erase(it);
    }
}

std::vector<SCEHandle<Light> > SCEScene::FindLightsInRange(const glm::vec3 &worldPosition)
{
    Internal::Log("TODO : finds actual lights in range");
    return s_scene->mLights;
}

/*** Debug functions ***/
void debugDeferredLighting(SCE_GBuffer &mLightingGBuffer){

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mLightingGBuffer.BindForLightPass();

    GLsizei width = SCECore::GetWindowWidth();
    GLsizei height = SCECore::GetWindowHeight();

    mLightingGBuffer.SetReadBuffer(SCE_GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void printGLState()
{

    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

    Debug::Log("GL_DEPTH_WRITEMASK " + std::to_string(depthMask));
    //Debug::Log("GL_DEPTH_FUNC" + std::to_string(glGet(GL_DEPTH_FUNC)));
    Debug::Log("GL_DEPTH_TEST " + std::to_string(glIsEnabled(GL_DEPTH_TEST)));

}

void debugDepthPixels(SCE_GBuffer &mLightingGBuffer)
{
    //mLightingGBuffer.BindForStencilPass();

    GLsizei width = SCECore::GetWindowWidth();
    GLsizei height = SCECore::GetWindowHeight();
    //GLuint* pixels = new GLuint[width * height];
    GLfloat* pixels = new GLfloat[width * height];

    glReadPixels(
        0,
        0,
        width,
        height,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,//GL_UNSIGNED_INT,
        pixels);

    string pixelStr = "";
    for ( int i = 0; i < height; ++i)
    {
        for ( int j = 0; j < width; ++j )
        {
            float pixel = pixels[(width * i) + j];
            string str = std::to_string(pixel);
            str.erase ( str.find_last_not_of('0') + 1, std::string::npos );
            pixelStr += str + ", ";
        }
        pixelStr += "\n";
    }

    Debug::Log(pixelStr);
    Debug::Log("\n //////////////////////////////////////////////// \n");
}

void debugStencilPixels()
{
    //mLightingGBuffer.BindForStencilPass();

    GLsizei width = SCECore::GetWindowWidth();
    GLsizei height = SCECore::GetWindowHeight();
    //GLuint* pixels = new GLuint[width * height];
    GLuint* pixels = new GLuint[width * height];

    glReadPixels(
        0,
        0,
        width,
        height,
        GL_STENCIL_INDEX,//GL_DEPTH_COMPONENT,
        GL_UNSIGNED_INT,
        pixels);

    string pixelStr = "";
    for ( int i = 0; i < height; ++i)
    {
        for ( int j = 0; j < width; ++j )
        {
            unsigned int pixel = pixels[(width * i) + j];
            string str = std::to_string(pixel);
            pixelStr += str + ", ";
        }
        pixelStr += "\n";
    }

    Debug::Log(pixelStr); 
}
/*** end of debug ***/

void SCEScene::renderSceneWithCamera(const SCEHandle<Camera> &camera)
{
    renderGeometryPass(camera);

    //debugPixels(mLightingGBuffer);


    //Render lights with stencil test and no writting to depth buffer
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);

    for(size_t i = 0; i < mLights.size(); ++i)
    {
        renderLightStencilPass(camera, mLights[i]);
        renderLightPass(camera, mLights[i]);
    }

    glDisable(GL_STENCIL_TEST);

    //Render final image from GBuffer to window framebuffer
    mLightingGBuffer.BindForFinalPass();

    GLsizei width = SCECore::GetWindowWidth();
    GLsizei height = SCECore::GetWindowHeight();

    //TODO hook post-processing pipeline here
    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    //reset to default framebufffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SCEScene::renderGeometryPass(const SCEHandle<Camera> &camera)
{
    //render objects without lighting
    mLightingGBuffer.BindForGeometryPass();
    // The geometry pass updates the depth buffer
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<MeshRenderer>()
           && camera->IsLayerRendered( mContainers[i]->GetLayer() ))
        {
            SCEHandle<Material> mat = mContainers[i]->GetComponent<Material>();
            // Use the shader
            mat->BindMaterialData();

            SCEHandle<MeshRenderer> renderer = mContainers[i]->GetComponent<MeshRenderer>();
            renderer->Render(camera);
        }
    }
}

void SCEScene::renderLightStencilPass(const SCEHandle<Camera>& camera, SCEHandle<Light> light)
{
    //use empty shader for stencil pass (only need to do VS for depth test)
    SCELighting::StartLightStencilPass();
    mLightingGBuffer.BindForStencilPass();

    //avoid writting in color and depth buffers in stencyl pass
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

    light->RenderForStencil(camera);

    //re enable color writting
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void SCEScene::renderLightPass(const SCEHandle<Camera>& camera, SCEHandle<Light> light)
{
    SCELighting::StartLightRenderPass();
    mLightingGBuffer.BindForLightPass();
    mLightingGBuffer.BindTexturesToLightShader();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
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

void SCEScene::resetClearColorToDefault()
{
    glClearColor(mDefaultClearColor.r, mDefaultClearColor.g, mDefaultClearColor.b, mDefaultClearColor.a);
}
