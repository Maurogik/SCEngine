/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Component.cpp ********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Component.hpp"

using namespace SCE;


Component::Component()
{

}

Component::~Component()
{

}

Container *Component::GetContainer()
{
    return mContainer;
}

void Component::SetContainer(Container * container)
{
    SCE_DEBUG_LOG("Setting container \n");
    mContainer = container;
}
