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

using namespace SCE;
using namespace std;


Scene* Scene::s_scene = 0l;

SCE::Scene::Scene()
{

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
    for(size_t i = 0; i < mContainers.size(); ++i){
        SECURE_DELETE(mContainers[i]);
    }
    mContainers.clear();
}

void SCE::Scene::CreateEmptyScene()
{
    SCE_ASSERT(s_scene == 0l
               , "A scene is already loaded, destroy it befor loading a new one")
    s_scene = new Scene();
}

void SCE::Scene::LoadScene(const std::string& scenePath)
{
    SCE_ASSERT(s_scene == 0l
               , "A scene is already loaded, destroy it befor loading a new one")
}


void SCE::Scene::Run()
{
    SCE_ASSERT(s_scene
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
        Camera* cam = GET_COMPONENT_FROM(mContainers[i], Camera);
        if(cam) {
            renderSceneWithCamera(cam);
        }
    }
}

void Scene::UpdateScene()
{
    //update game clock
    SCETime::Update();

    //Duplicate containers before updating because
    //the list might be modified during the updates

    vector<Container*> tmpContainers = s_scene->mContainers;

    for(size_t i = 0; i < tmpContainers.size(); ++i){
        const vector<GameObject*> gos = tmpContainers[i]->GetGameObjects();
        for(size_t h = 0; h < gos.size(); ++h){
            gos[h]->Update();
        }
    }

    /**
     * Temporary
     **/
    SCE_DEBUG_LOG("move control code to appropriate location");

    Component* comp;
    for(size_t i = 0; i < mContainers.size(); ++i){
        comp = GET_COMPONENT_FROM(mContainers[i], Camera);
        if(comp) {
            break;
        }
    }
    GLFWwindow* window = SCECore::GetWindow();
    Transform* transform = GET_COMPONENT_FROM(comp->GetContainer(), Transform);
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

void SCE::Scene::DestroyScene()
{
    SECURE_DELETE(s_scene);
}

void SCE::Scene::AddContainer(Container *obj)
{
    SCE_DEBUG_LOG("Adding container to scene");
    s_scene->mContainers.push_back(obj);
}

void Scene::RemoveContainer(Container *obj)
{
    vector<Container*>::iterator it;
    for(it = s_scene->mContainers.begin(); it != s_scene->mContainers.end(); ++it){
        if(*it == obj) {
            s_scene->mContainers.erase(it);
        }
    }
}

std::vector<Container *> Scene::FindContainersWithTag(const string &tag)
{
    vector<Container*> tagged;
    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->GetTag() == tag){
            tagged.push_back(s_scene->mContainers[i]);
        }
    }
    return tagged;
}

std::vector<Container *> Scene::FindContainersWithLayer(const string &layer)
{
    vector<Container*> layered;
    for(size_t i = 0; i < s_scene->mContainers.size(); ++i){
        if(s_scene->mContainers[i]->GetLayer() == layer){
            layered.push_back(s_scene->mContainers[i]);
        }
    }
    return layered;
}

void Scene::RegisterLight(Light *light)
{
    if(find(s_scene->mLights.begin(), s_scene->mLights.end(), light)== s_scene->mLights.end()){
       s_scene-> mLights.push_back(light);
    }
}

void Scene::UnregisterLight(Light *light)
{
    vector<Light*>::iterator it = find(s_scene->mLights.begin(), s_scene->mLights.end(), light);
    if(it != s_scene->mLights.end()){
        s_scene->mLights.erase(it);
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

void Scene::renderSceneWithCamera(Camera *camera)
{
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(camera->IsLayerRendered( mContainers[i]->GetLayer() )){
            MeshRenderer* renderer = GET_COMPONENT_FROM(mContainers[i], MeshRenderer);
            if(renderer) {
                renderer->Render(camera);
            }
        }
    }
}
