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


Component::Component(Handle<Container> &container, const string &typeName, const int &guid)
    : mContainer(container)
    , mTypeHash(SCE::Tools::HashFromString( "Component::"+typeName))
{

}

Component::~Component()
{

}

Handle<Container> Component::GetContainer()
{
    return mContainer;
}

const Handle<Container> Component::GetContainer() const
{
    return mContainer;
}

const int& Component::GetTypeHash() const
{
    return mTypeHash;
}
