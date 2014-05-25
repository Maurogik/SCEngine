/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.cpp ********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Component.hpp"
#include "../headers/Scene.hpp"

using namespace SCE;
using namespace std;


Container::Container()
    : mTag(DEFAULT_TAG)
    , mLayer(DEFAULT_LAYER)
{
    SCE_DEBUG_LOG("New container");
    Scene::AddContainer(this);
}

Container::~Container()
{
    //destroy map & contained components
    map<string, Component*>::iterator map_it;
    for(map_it = mComponentMap.begin(); map_it != mComponentMap.end(); ++map_it) {
        Component * comp = mComponentMap[map_it->first];
        SECURE_DELETE(comp);
    }
    mComponentMap.clear();
}

void Container::AddComponent(string componentName, Component *component)
{
    SCE_ASSERT(mComponentMap.count(componentName) == 0, "Component already exists");

    SCE_DEBUG_LOG("Adding component %s", componentName.c_str());

    mComponentMap[componentName] = component;
}

Component *Container::GetComponent(string componentName)
{
    /*SCE_ASSERT(mComponentMap.count(componentName) != 0, "Component %s not in container"
               , componentName.c_str());*/
    return mComponentMap[componentName];
}

void Container::RemoveComponent(string componentName)
{
    Component * comp = mComponentMap[componentName];
    SECURE_DELETE(comp);
    mComponentMap.erase(mComponentMap.find((componentName)));
}

string Container::GetTag()
{
    return mTag;
}

string Container::GetLayer()
{
    return mLayer;
}

void Container::SetTag(string tag)
{
    mTag = tag;
}

void Container::SetLayer(string layer)
{
    mLayer = layer;
}
