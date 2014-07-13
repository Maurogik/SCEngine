#include "Rotator.hpp"

using namespace SCE;

Rotator::Rotator() : mSpeed(100.0f)
{

}

Rotator::~Rotator()
{

}

void Rotator::Update()
{
    Transform *transform = GET_COMPONENT(Transform);
    //transform->RotateAroundAxis(vec3(0, 0, 1), mSpeed * SCETime::deltaTime());
    transform->RotateAroundPivot(vec3(-3, 0, 0), vec3(0, 0, 1), mSpeed * SCETime::deltaTime());
    //SCE_DEBUG_LOG("dt : %f", SCETime::timeInSeconds());
}
