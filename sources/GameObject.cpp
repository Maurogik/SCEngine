/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : GameObject.cpp ********/
/**************************************/

#include "../headers/GameObject.hpp"
#include "../headers/Container.hpp"
#include "../headers/Scene.hpp"

using namespace SCE;
using namespace std;


GameObject::GameObject(Handle<Container> &container, const string &typeName)
    : Component(container, "GameObject::" + typeName)
{
    Scene::RegisterGameObject(Handle<GameObject>(this));
}

GameObject::~GameObject()
{
    Scene::UnregisterGameObject(Handle<GameObject>(this));
}
