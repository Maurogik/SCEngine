/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.cpp ********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Component.hpp"
#include "../headers/Scene.hpp"
#include "../headers/SCEInternal.hpp"

using namespace SCE;
using namespace std;


Container::Container(const string &name)
    : mComponents()
    , mTag(DEFAULT_TAG)
    , mLayer(DEFAULT_LAYER)
    , mName(name)
{
    SCEInternal::InternalMessage("New container");
    Scene::AddContainer(shared_ptr<Container>(this));
}

Container::~Container()
{
}

const string& Container::GetTag() const
{
    return mTag;
}

const string& Container::GetLayer() const
{
    return mLayer;
}

void Container::SetTag(const string& tag)
{
    mTag = tag;
}

void Container::SetLayer(const string& layer)
{
    mLayer = layer;
}
const string& Container::GetName() const
{
    return mName;
}

void Container::SetName(const std::string &name)
{
    mName = name;
}



