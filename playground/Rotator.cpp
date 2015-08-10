#include "Rotator.hpp"

using namespace SCE;
using namespace std;

Rotator::Rotator(SCE::SCEHandle<Container> container)
    : GameObject(container, "Rotator"), mSpeed(25.0f)
{

}

void Rotator::Update()
{
    //static bool start = true;
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    /*if(start){
        start = false;
        transform->RotateAroundAxis(vec3(0, 0, 1), 90);
    }*/

    transform->RotateAroundAxis(vec3(0, 0, 1), mSpeed * SCETime::DeltaTime());
    vec3 worldPos = transform->GetWorldPosition();
    worldPos += vec3(0.0f, 0.0f, 0.50f) * (float)SCETime::DeltaTime();
    //transform->SetWorldPosition(worldPos);

    /*transform->RotateAroundAxis(vec3(0, 1, 0), mSpeed * SCETime::DeltaTime());
    transform->RotateAroundAxis(vec3(1, 0, 0), mSpeed * SCETime::DeltaTime());
    transform->RotateAroundPivot(vec3(0, 0, 2), vec3(0, 0, 1), mSpeed * SCETime::DeltaTime());*/
}
