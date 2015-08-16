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

LookAtTarget::LookAtTarget(SCE::SCEHandle<SCE::Container> container, const vec3& target)
    : SCE::GameObject(container, "LookAtTarget"),
      mWorldTarget(target)
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

