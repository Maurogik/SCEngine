/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : GameObject.cpp ********/
/**************************************/

#include "../headers/GameObject.hpp"
#include "../headers/Container.hpp"

using namespace SCE;
using namespace std;


GameObject::GameObject()
{

}

GameObject::~GameObject()
{
    if(GetContainer()){
        GetContainer()->RemoveGameObject(this);
    }
}

void GameObject::SetContainer(Container *cont)
{
    SCE_DEBUG_LOG("Setting GameObject container");
    Component::SetContainer(cont);
    if(GetContainer()){
        GetContainer()->AddGameObject(this);
    }
}
