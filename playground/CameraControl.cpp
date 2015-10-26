#include "CameraControl.hpp"
#include "../headers/SCECore.hpp"

using namespace SCE;
using namespace std;


CameraControl::CameraControl(SCE::SCEHandle<Container> container, SCE::SCEHandle<Transform> target)
    : GameObject(container, "CamControl"), mTarget(target), mDistanceFromTarget(0.0f, 1.0f, -3.5f),
      mLookAheadTarget(0.0f, 1.0f, 100.0f)
{  
    mAverageOffset = mTarget->Forward()*mLookAheadTarget.z + mTarget->Up()*mLookAheadTarget.y;
}

void CameraControl::Update()
{
    //move camera
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();
    glm::vec3 targetPos = mTarget->Forward()*mDistanceFromTarget.z + mTarget->Up()*mDistanceFromTarget.y;
    targetPos += mTarget->GetWorldPosition();
    transform->SetWorldPosition(targetPos);

    glm::vec3 lookAtTarget = mTarget->Forward()*mLookAheadTarget.z + mTarget->Up()*mLookAheadTarget.y;

    float avgDuration = 0.2f;
    float deltaTime = SCE::Time::DeltaTime();
    mAverageOffset = (lookAtTarget*deltaTime + mAverageOffset*avgDuration)
            / (avgDuration + deltaTime);

    transform->LookAt(mAverageOffset + mTarget->GetWorldPosition());
}

