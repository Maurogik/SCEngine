/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:LookAtTarget.cpp*********/
/**************************************/
#include "LookAtTarget.hpp"
//#include "../headers/SCE.hpp"

using namespace SCE;
using namespace glm;
using namespace std;

LookAtTarget::LookAtTarget(SCE::SCEHandle<SCE::Container> container)
    : SCE::GameObject(container, "LookAtTarget"),
      mWorldTarget(0.0f, 0.0f, 0.0f)
{

}

LookAtTarget::~LookAtTarget()
{

}

void LookAtTarget::Update()
{
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();
    transform->LookAt(mWorldTarget);
}

