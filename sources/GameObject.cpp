/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : GameObject.cpp ********/
/**************************************/

#include "../headers/GameObject.hpp"
#include "../headers/Container.hpp"
#include "../headers/SCEScene.hpp"

using namespace SCE;
using namespace std;


GameObject::GameObject(SCEHandle<Container> &container, const string &typeName)
    : Component(container, "GameObject::" + typeName)
{
    SCEScene::RegisterGameObject(SCEHandle<GameObject>(this));
}

GameObject::~GameObject()
{
    SCEScene::UnregisterGameObject(SCEHandle<GameObject>(this));
}
