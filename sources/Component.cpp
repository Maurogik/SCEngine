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


Component::Component(Container& container, const string &typeName)
    : mContainer(container)
    , mTypeHash(SCE::Tools::HashFromString( "Component::"+typeName))
{

}

Component::~Component()
{

}

Container &Component::GetContainer()
{
    return mContainer;
}

const Container &Component::GetContainer() const
{
    return mContainer;
}

const int& Component::GetTypeHash() const
{
    return mTypeHash;
}
