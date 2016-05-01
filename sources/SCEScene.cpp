/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Scene.cpp **********/
/**************************************/

#include "../headers/SCEScene.hpp"
#include "../headers/MeshRenderer.hpp"
#include "../headers/Camera.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCETerrain.hpp"

#include "../headers/SCECore.hpp"

using namespace SCE;
using namespace std;


SCEScene* SCEScene::s_scene = nullptr;

SCE::SCEScene::SCEScene()
    : mContainers(), mGameObjects(), mLastId(0)
{    
}

SCE::SCEScene::~SCEScene()
{
    SCE::Terrain::Cleanup();
    Internal::Log("Delete scene");
    Internal::Log("Clear stuff");
    for(auto cont : mContainers)
    {
        delete(cont);
    }
    mContainers.clear();
    mGameObjects.clear();
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

void SCEScene::AddTerrain(float terrainSize, float patchSize, float baseHeight)
{
    SCE::Terrain::Init(terrainSize, patchSize, baseHeight);
}

void SCEScene::RemoveTerrain()
{
    SCE::Terrain::Cleanup();
}

#ifdef SCE_DEBUG_ENGINE
void SCEScene::ReloadAllMaterials()
{
    for(Container* container : s_scene->mContainers)
    {
        if(container->HasComponent<Material>())
        {
            container->GetComponent<Material>()->ReloadMaterial();
        }
    }
}
#endif

void SCEScene::renderSceneWithCamera(const SCEHandle<Camera> &camera)
{
    vector<Container*> objectsToRender;
    for(size_t i = 0; i < mContainers.size(); ++i){
        if(mContainers[i]->HasComponent<MeshRenderer>()
           && camera->IsLayerRendered( mContainers[i]->GetLayer() ))
        {
            objectsToRender.push_back(mContainers[i]);
        }
    }

    SCE::Render::Render(camera, objectsToRender);
}



