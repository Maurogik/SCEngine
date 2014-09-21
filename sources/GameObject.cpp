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


GameObject::GameObject(Container &container, const string &typeName)
    : Component(container, "GameObject::" + typeName)
{
    Scene::RegisterGameObject(shared_ptr<GameObject>(this));
}

GameObject::~GameObject()
{
    Scene::UnregisterGameObject(shared_ptr<GameObject>(this));
}
