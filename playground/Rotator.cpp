#include "Rotator.hpp"

using namespace SCE;
using namespace std;

Rotator::Rotator(SCE::Handle<Container> container)
    : GameObject(container, "Rotator"), mSpeed(25.0f)
{

}

Rotator::~Rotator()
{

}

void Rotator::Update()
{
    static bool start = true;
    Handle<Transform> transform = GetContainer()->GetComponent<Transform>();

    /*if(start){
        start = false;
        transform->RotateAroundAxis(vec3(0, 0, 1), 90);
    }*/

    transform->RotateAroundAxis(vec3(0, 0, 1), mSpeed * SCETime::deltaTime());
    transform->RotateAroundAxis(vec3(0, 1, 0), mSpeed * SCETime::deltaTime());
    transform->RotateAroundAxis(vec3(1, 0, 0), mSpeed * SCETime::deltaTime());
    transform->RotateAroundPivot(vec3(0, 0, 2), vec3(0, 0, 1), mSpeed * SCETime::deltaTime());
}
