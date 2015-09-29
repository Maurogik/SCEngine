#include "PlayerControl.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCEDebugText.hpp"

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
}

void PlayerControl::Update()
{
    //move player

    GLFWwindow* window = SCECore::GetWindow();
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    double xMouse = 0.0;
    double yMouse = 0.0;

    glfwGetCursorPos(window, &xMouse, &yMouse);

    xMouse /= SCECore::GetWindowWidth();
    yMouse /= SCECore::GetWindowHeight();

    float dX = xMouse - 0.5f;
    float dY = yMouse - 0.5f;

    float deltaTime = SCE::Time::DeltaTime();
    float deltaGoodness = SCE::Math::mapToRange(0.010, 0.033, 0.0, 1.0, deltaTime);
    glm::vec3 printColor = glm::vec3(deltaGoodness, 1.0 - deltaGoodness, 0.0);
    SCE::DebugText::Print("FPS   : " + std::to_string(1.0f/deltaTime), printColor);
    SCE::DebugText::Print("Frame : " + std::to_string(deltaTime * 1000.0f) + " ms", printColor);

    float xRotateSpeed = 0.015f;
    float yRotateSpeed = 0.02f;
    float zRotateSpeed = 1.5f;
    float speed = 20.0f;
    float turnSlowdown = 10.0f;
    float diveSpeedIncrease = 30.0f;
    float climbSlowdown = 10.0f;

    vec3 position = transform->GetWorldPosition();
    vec3 forward = transform->Forward();    

    dX = 0.0f;
    dY = 0.0f;

    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS)
    {
        dY = -1.0f;
    }

    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
    {
        dY = 1.0f;
    }

    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
    {
        dX = 1.0f;
    }

    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
    {
       dX = -1.0f;
    }

    if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS)
    {
#ifdef SCE_DEBUG
       speed = 150.0f;
#else
       speed = 50.0f;
#endif
    }
    else if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS)
    {
#ifdef SCE_DEBUG
       speed = 1.0f;
#else
       speed = 5.0f;
#endif
    }


//    vec3 yAxis(transform->LocalToWorldDir(vec3(0.0, 1.0, 0.0)));
    vec3 yAxis(0.0, 1.0, 0.0);
    vec3 xAxis(transform->LocalToWorldDir(vec3(1.0, 0.0, 0.0)));
    xAxis.y = 0.0f;
    xAxis = normalize(xAxis);

    float avgDuration = 0.3f;
    float upStrAvgDuration = 0.5f;
    averageDx = (averageDx * avgDuration + dX * deltaTime) / (avgDuration + deltaTime);
    averageDy = (averageDy * avgDuration + dY * deltaTime) / (avgDuration + deltaTime);

    dX = averageDx * xRotateSpeed;
    dY = averageDy * yRotateSpeed;

    vec3 target = dX * xAxis +
            dY * yAxis +
            transform->Forward();

    float upLimit = 0.75f;
    float lowLimit = -0.85f;

    target.y = glm::clamp(target.y, lowLimit, upLimit);

    float upStr = averageDx / (1.0f + abs(target.y * 5.0f))  * zRotateSpeed;
    averageUpStr = (averageUpStr * upStrAvgDuration + upStr * deltaTime) / (upStrAvgDuration + deltaTime);

//    Debug::Log(std::to_string(upStr));
    vec3 upVector = normalize(averageUpStr * xAxis + vec3(0.0, 1.0, 0.0));

    float turnModifier = abs(averageDx) * turnSlowdown;
    float pitchModifier = target.y < 0.0f ? target.y * diveSpeedIncrease : target.y * climbSlowdown;
    speed = speed - turnModifier - pitchModifier;

    if(lastSpeed > speed)
    {//deccelerate slowly
        speed = (lastSpeed * 99.0f + speed)/100.0f;
    }
    lastSpeed = speed;

    target = transform->GetWorldPosition() + target;

    transform->LookAt(target, upVector);
    //constant mouvement
    position += forward * deltaTime * speed;
    transform->SetWorldPosition(position);

}
