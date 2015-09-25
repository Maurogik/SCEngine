#include "CameraControl.hpp"
#include "../headers/SCECore.hpp"

using namespace SCE;
using namespace std;

CameraControl::CameraControl(SCE::SCEHandle<Container> container, SCE::SCEHandle<Transform> target)
    : GameObject(container, "CamControl"), mTarget(target), mDistanceFromTarget(0.0f, 1.0f, -5.0f)
{
}

void CameraControl::Update()
{
    //move camera
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();
    glm::vec3 targetPos = mTarget->GetWorldPosition();
    glm::vec3 pos = targetPos;
    pos += mTarget->Forward() * -3.5f + mTarget->Up() * 1.0f;
    transform->SetWorldPosition(pos);

    float lookAheadDist = 10.0f;
    glm::vec3 lookAtTarget = mTarget->GetWorldPosition() + mTarget->Forward() * lookAheadDist +
            mTarget->Up();
    transform->SmoothLookAt(lookAtTarget, 0.3f);
}

