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

using namespace SCE;
using namespace std;


SCEScene* SCEScene::s_scene = 0l;

SCE::SCEScene::SCEScene()
    : mLastId(0)
{
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LEQUAL);
    //glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);

}

SCE::SCEScene::~SCEScene()
{
    SCEInternal::InternalMessage("Delete scene");
    SCEInternal::InternalMessage("Clear stuff");
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
    Debug::Assert(s_scene == 0l
               , "A scene is already loaded, destroy it befor loading a new one");
    s_scene = new SCEScene();
}

void SCE::SCEScene::LoadScene(const std::string& scenePath)
{
    Debug::Assert(s_scene == 0l
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

    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<Camera>()){
            SCEHandle<Camera> cam = mContainers[i]->GetComponent<Camera>();
            renderSceneWithCamera(cam);
        }
    }
}

void SCEScene::UpdateScene()
{
    //update game clock
    SCETime::Update();

    //TODO SCEHandle case where an object is waiting to be destroyed

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
    vec3 position = transform->GetLocalPosition();

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    lastTime = currentTime;

    float rotateSpeed = 45.0f;
    float speed = 5.0f;

    //vec3 forward(0, 0, 1);
    //vec3 left(1, 0 ,0);

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

    if(glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS){
        transform->RotateAroundAxis(vec3(0, 1, 0), rotateSpeed * deltaTime);
    }

    if(glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
        transform->RotateAroundAxis(vec3(0, 1, 0), -rotateSpeed * deltaTime);
    }

    transform->SetLocalPosition(position);
    /** End of temporary**/
}

void SCE::SCEScene::DestroyScene()
{
    SECURE_DELETE(s_scene);
}

SCEHandle<Container> SCEScene::CreateContainer(const string &name)
{
    if(!s_scene){
        Debug::PrintError("No scene to add the container to, create a scene first");
    }
    Container* cont = new Container(name, ++s_scene->mLastId);
    s_scene->mContainers.push_back(cont);
    return SCEHandle<Container>(cont);
}

void SCEScene::DestroyContainer(const SCEHandle<Container> &container)
{
    RemoveContainer(container->GetContainerId());
}


void SCEScene::RemoveContainer(const int &objId)
{
    SCEInternal::InternalMessage("Removing container from scene");
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
{   //this should also call every material to run InitLightRenderData with their shaders
    if(find(begin(s_scene->mLights), end(s_scene->mLights), light) == end(s_scene->mLights)){
       s_scene->mLights.push_back(light);
    }

    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->HasComponent<Material>()){
            SCEHandle<Material> mat = s_scene->mContainers[i]->GetComponent<Material>();
            light->InitRenderDataForShader(mat->GetShaderProgram());
        }
    }
}

void SCEScene::UnregisterLight(SCEHandle<Light> light)
{
    auto it = find(begin(s_scene->mLights), end(s_scene->mLights), light);
    if(it != end(s_scene->mLights)){
        s_scene->mLights.erase(it);
    }
}

void SCEScene::InitLightRenderData(const GLuint &shaderId)
{
    //TODO store light uniforms for shader only once, not for each light
    for(size_t i = 0; i < s_scene->mLights.size(); ++i){
        s_scene->mLights[i]->InitRenderDataForShader(shaderId);
    }
}

void SCEScene::BindLightRenderData(const GLuint &shaderId)
{
    //here I should test to see if the lights will influence the object rendering
    for(size_t i = 0; i < s_scene->mLights.size(); ++i){
        s_scene->mLights[i]->BindRenderDataForShader(shaderId);
        s_scene->mLights[i]->BindLightModelForShader(shaderId);
    }
}

std::vector<SCEHandle<Light> > SCEScene::FindLightsInRange(const glm::vec3 &worldPosition)
{
    return s_scene->mLights;
}

void SCEScene::renderSceneWithCamera(const SCEHandle<Camera> &camera)
{
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<MeshRenderer>()
           && camera->IsLayerRendered( mContainers[i]->GetLayer() )){
            SCEHandle<MeshRenderer> renderer = mContainers[i]->GetComponent<MeshRenderer>();
            renderer->Render(camera);
        }
    }
}
