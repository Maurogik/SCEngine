/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Component.cpp ********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Component.hpp"
#include "../headers/SCETools.hpp"

using namespace SCE;
using namespace std;


Component::Component(SCEHandle<Container> &container, const string &typeName, const int &guid)
    : mContainer(container)
    , mTypeHash(SCE::Tools::HashFromString( "Component::"+typeName))
    , mIsHidden(false)
{

}

Component::~Component()
{

}

SCEHandle<Container> Component::GetContainer()
{
    return mContainer;
}

const SCEHandle<Container> Component::GetContainer() const
{
    return mContainer;
}

const int& Component::GetTypeHash() const
{
    return mTypeHash;
}

void Component::SetIsHidden(bool isHidden)
{
    mIsHidden = isHidden;
}

bool Component::IsHidden()
{
    return mIsHidden;
}
