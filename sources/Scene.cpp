/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Scene.cpp **********/
/**************************************/

#include "../headers/Scene.hpp"
#include "../headers/MeshRenderer.hpp"
#include "../headers/Camera.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/GameObject.hpp"
#include "../headers/SCEInternal.hpp"

using namespace SCE;
using namespace std;


Scene* Scene::s_scene = 0l;
bool   Scene::mCleaningScene = false;

SCE::Scene::Scene()
{
    mCleaningScene = false;
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

}

SCE::Scene::~Scene()
{
}

void SCE::Scene::CreateEmptyScene()
{
    Debug::Assert(s_scene == 0l
               , "A scene is already loaded, destroy it befor loading a new one");
    s_scene = new Scene();
}

void SCE::Scene::LoadScene(const std::string& scenePath)
{
    Debug::Assert(s_scene == 0l
               , "A scene is already loaded, destroy it befor loading a new one");
}


void SCE::Scene::Run()
{
    Debug::Assert(s_scene
               , "There is no scene to display, create or load a scene before running the engine");
    s_scene->UpdateScene();
    s_scene->RenderScene();
}


void SCE::Scene::RenderScene()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Parse cameras and render them

    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<Camera>()){
            Camera& cam = mContainers[i]->GetComponent<Camera>();
            renderSceneWithCamera(cam);
        }
    }
}

void Scene::UpdateScene()
{
    //update game clock
    SCETime::Update();

    //TODO handle case where an object is waiting to be destroyed

    vector<shared_ptr<GameObject> > tmpGameObjects = s_scene->mGameObjects;

    for(size_t i = 0; i < tmpGameObjects.size(); ++i){
        tmpGameObjects[i]->Update();
    }

    /************************************************************
     * Temporary
     **/
    SCEInternal::InternalMessage("move control code to appropriate location");

    shared_ptr<Container> contPtr;
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<Camera>()){
            contPtr = mContainers[i];
            break;
        }
    }

    if(!contPtr)
        return;


    GLFWwindow* window = SCECore::GetWindow();
    Transform& transform = contPtr->GetComponent<Transform>();
    vec3 position = transform.GetLocalPosition();

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

    vec3 forward = transform.Forward();
    vec3 left = transform.Left();

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
        transform.RotateAroundAxis(vec3(0, 1, 0), rotateSpeed * deltaTime);
    }

    if(glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
        transform.RotateAroundAxis(vec3(0, 1, 0), -rotateSpeed * deltaTime);
    }

    transform.SetLocalPosition(position);
    /** End of temporary**/
}

void SCE::Scene::DestroyScene()
{
    mCleaningScene = true;
    SECURE_DELETE(s_scene);
}

void SCE::Scene::AddContainer(std::shared_ptr<Container> obj)
{
    SCEInternal::InternalMessage("Adding container to scene");
    s_scene->mContainers.push_back(obj);
}

void Scene::RemoveContainer(std::shared_ptr<Container> obj)
{
    SCEInternal::InternalMessage("Removing container from scene");
    auto objIt = find(begin(s_scene->mContainers), end(s_scene->mContainers), obj);
    if(objIt != end(s_scene->mContainers)) {
        s_scene->mContainers.erase(objIt);
    }
}

vector<shared_ptr<Container> > Scene::FindContainersWithTag(const string &tag)
{
    vector<shared_ptr<Container> > tagged;
    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->GetTag() == tag){
            tagged.push_back(s_scene->mContainers[i]);
        }
    }
    return tagged;
}

vector<shared_ptr<Container> >  Scene::FindContainersWithLayer(const string &layer)
{
    vector<shared_ptr<Container> >  layered;
    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->GetLayer() == layer){
            layered.push_back(s_scene->mContainers[i]);
        }
    }
    return layered;
}

void Scene::RegisterGameObject(std::shared_ptr<GameObject> gameObject)
{
    if(find(begin(s_scene->mGameObjects), end(s_scene->mGameObjects), gameObject) == end(s_scene->mGameObjects)){
       s_scene-> mGameObjects.push_back(gameObject);
    }
}

void Scene::UnregisterGameObject(std::shared_ptr<GameObject> gameObject)
{
    if(!mCleaningScene){
        auto it = find(begin(s_scene->mGameObjects), end(s_scene->mGameObjects), gameObject);
        if(it != end(s_scene->mGameObjects)){
            s_scene-> mGameObjects.erase(it);
        }
    }
}

void Scene::RegisterLight(shared_ptr<Light> light)
{
    if(find(begin(s_scene->mLights), end(s_scene->mLights), light) == end(s_scene->mLights)){
       s_scene-> mLights.push_back(light);
    }
}

void Scene::UnregisterLight(shared_ptr<Light> light)
{
    if(!mCleaningScene) {
        auto it = find(begin(s_scene->mLights), end(s_scene->mLights), light);
        if(it != end(s_scene->mLights)){
            s_scene->mLights.erase(it);
        }
    }
}

void Scene::InitLightRenderData(const GLuint &shaderId)
{
    for(size_t i = 0; i < s_scene->mLights.size(); ++i){
        s_scene->mLights[i]->InitRenderDataForShader(shaderId);
    }
}

void Scene::BindLightRenderData(const GLuint &shaderId)
{
    for(size_t i = 0; i < s_scene->mLights.size(); ++i){
        s_scene->mLights[i]->BindRenderDataForShader(shaderId);
    }
}

void Scene::renderSceneWithCamera(const Camera &camera)
{
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<MeshRenderer>()
           && camera.IsLayerRendered( mContainers[i]->GetLayer() )){
            MeshRenderer& renderer = mContainers[i]->GetComponent<MeshRenderer>();
            renderer.Render(camera);
        }
    }
}
