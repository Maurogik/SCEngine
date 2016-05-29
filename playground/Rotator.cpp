#include "Rotator.hpp"

using namespace SCE;
using namespace std;

Rotator::Rotator(SCE::SCEHandle<Container> container, float speed, vec3 axis)
    : GameObject(container, "Rotator"), mSpeed(speed), mRotationAxis(axis)
{
}

void Rotator::Update()
{
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    transform->RotateAroundAxis(mRotationAxis, mSpeed * float(SCE::Time::DeltaTime()));
}
