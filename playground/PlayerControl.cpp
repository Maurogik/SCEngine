#include "PlayerControl.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCEDebugText.hpp"
#include "../headers/SCETerrain.hpp"
#include "../headers/SCEInput.hpp"

using namespace SCE;
using namespace std;

PlayerControl::PlayerControl(SCE::SCEHandle<Container> container)
    : GameObject(container, "CamControl")
{
    double xMouse = 0.0;
    double yMouse = 0.0;
    GLFWwindow* window = SCECore::GetWindow();
    glfwGetCursorPos(window, &xMouse, &yMouse);

    xMouse /= SCECore::GetWindowWidth();
    yMouse /= SCECore::GetWindowHeight();

    averageDx = 0.0f;
    averageDy = 0.0f;
    averageUpStr = 1.0f;
    lastSpeed = 0.0f;
    averageHeight = 0.0f;

    mTransform = GetContainer()->GetComponent<Transform>();
    glm::vec3 pos = mTransform->GetScenePosition();
    pos.y = SCE::Terrain::GetTerrainHeight(pos) + 20.0f;
    mTransform->SetScenePosition(pos);
}

void PlayerControl::Update()
{
    //move player

    GLFWwindow* window = SCECore::GetWindow();    

    double xMouse = 0.0;
    double yMouse = 0.0;

    glfwGetCursorPos(window, &xMouse, &yMouse);

    xMouse /= SCECore::GetWindowWidth();
    yMouse /= SCECore::GetWindowHeight();

    float dX = float(xMouse) - 0.5f;
    float dY = float(yMouse) - 0.5f;

    float deltaTime = float(SCE::Time::DeltaTime());
    float deltaGoodness = SCE::Math::MapToRange(0.010f, 0.050f, 0.0f, 1.0f, deltaTime);
    glm::vec3 printColor = glm::vec3(deltaGoodness, 1.0 - deltaGoodness, 0.0);
    SCE::DebugText::LogMessage("FPS   : " + std::to_string(1.0f/deltaTime), printColor);
    SCE::DebugText::LogMessage("Frame : " + std::to_string(deltaTime*1000.0f) + " ms", printColor);

//    float xRotateSpeed = 0.015f;
//    float yRotateSpeed = 0.015f;
    float xRotateSpeed = 1.0f;
    float yRotateSpeed = 1.0f;
    float zRotateSpeed = 1.65f;
    float speed = 30.0f;
    float diveSpeedIncrease = 50.0f;
    float climbSlowdown = 10.0f;

    dX = 0.0f;
    dY = 0.0f;

    using namespace SCE::Input;

    if (GetKeyAction( GLFW_KEY_UP ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_Z ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_W ) == KeyAction::Hold)
    {
        dY = -1.0f;
    }

    if (GetKeyAction( GLFW_KEY_DOWN ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_S ) == KeyAction::Hold)
    {
        dY = 1.0f;
    }

    if (GetKeyAction( GLFW_KEY_RIGHT ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_D ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_D ) == KeyAction::Hold)
    {
        dX = 1.0f;
    }

    if (GetKeyAction( GLFW_KEY_LEFT ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_Q ) == KeyAction::Hold ||
        GetKeyAction( GLFW_KEY_A ) == KeyAction::Hold)
    {
       dX = -1.0f;
    }

    if (GetKeyAction( GLFW_KEY_LEFT_SHIFT ) == KeyAction::Hold)
    {
#ifndef SCE_FINAL
       speed = 500.0f;
#else
       speed = 80.0f;
#endif
    }
    else if (GetKeyAction( GLFW_KEY_LEFT_CONTROL ) == KeyAction::Hold)
    {
        speed = 500.0f;
    }
    else if (GetKeyAction( GLFW_KEY_SPACE ) == KeyAction::Hold)
    {
#ifndef SCE_FINAL
       speed = 0.0f;
       lastSpeed = 0.0f;
       diveSpeedIncrease = 0.0f;
       climbSlowdown = 0.0f;
#else
//       speed = 5.0f;
        speed = 10.0f;
        lastSpeed *= 0.9f;
#endif
    }

    vec3 yAxis(0.0, 1.0, 0.0);
    vec3 xAxis(mTransform->LocalToSceneDir(vec3(1.0, 0.0, 0.0)));
    xAxis.y = 0.0f;
    xAxis = normalize(xAxis);

    float avgDuration = 0.8f;
    float upStrAvgDuration = 0.3f;
    averageDx = (averageDx*avgDuration + dX*deltaTime)/(avgDuration + deltaTime);
    averageDy = (averageDy*avgDuration + dY*deltaTime)/(avgDuration + deltaTime);

    dX = averageDx*xRotateSpeed*deltaTime;
    dY = averageDy*yRotateSpeed*deltaTime;

    vec3 target = dX*xAxis + dY*yAxis + mTransform->Forward();

    target.y = glm::clamp(target.y, -0.85f, 0.75f);

    //make eagle take sharp turns by rotating the up vector
    float upStr = averageDx / (1.0f + abs(target.y*5.0f));
    averageUpStr = (averageUpStr*upStrAvgDuration + upStr*deltaTime)/(upStrAvgDuration + deltaTime);
    vec3 upVector = normalize(averageUpStr*xAxis*zRotateSpeed + vec3(0.0, 1.0, 0.0));

    //slowdown when going up, accelerate when going down
    float pitchModifier = target.y < 0.0f ? target.y*diveSpeedIncrease : target.y*climbSlowdown;
    speed = speed - pitchModifier;

    if(lastSpeed > speed)
    {//deccelerate slowly
        float speedAvgDuration = 3.0f;
        speed = (lastSpeed*speedAvgDuration + speed*deltaTime)/(speedAvgDuration + deltaTime);
    }
    lastSpeed = speed;


    SCE::DebugText::LogMessage("Mvt speed : " + std::to_string(speed));

    float turnStrength = glm::clamp(40.0f/speed, 0.1f, 2.0f);
    target = mTransform->GetScenePosition() + target * turnStrength;

    glm::vec3 rootPosition = SCEScene::GetFrameRootPosition();

    //how far above the ground we want to be
    float heightOffset = 4.0f;
    mTransform->LookAt(target, upVector);
    //constant mouvement
    vec3 position = mTransform->GetScenePosition();
    position += mTransform->Forward()*deltaTime*speed;
    //this height is worldspace, not scene space
    float height = SCE::Terrain::GetTerrainHeight(position + rootPosition);
    float heightAvgDur = 0.1f;
    //need to offset for scene root
    averageHeight = (averageHeight*heightAvgDur + height*deltaTime)/(heightAvgDur + deltaTime);
    if(position.y + rootPosition.y < averageHeight + heightOffset)
    {
        position.y = averageHeight + heightOffset - rootPosition.y;
    }
    mTransform->SetScenePosition(position);

    SCEScene::SetRootWorldspacePosition(position + rootPosition);
}
