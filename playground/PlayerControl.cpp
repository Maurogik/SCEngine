#include "PlayerControl.hpp"
#include "../headers/SCECore.hpp"

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
    float xRotateSpeed = 0.02f;
    float yRotateSpeed = 0.03f;
    float zRotateSpeed = 1.5f;
    float speed = 30.0f;
    float turnSlowdown = 10.0f;
    float diveSpeedIncrease = 40.0f;
    float climbSlowdown = 10.0f;

    vec3 position = transform->GetWorldPosition();
    vec3 forward = transform->Forward();    

    dX = 0.0f;
    dY = 0.0f;
    // Move forward
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS)
    {
        dY = -1.0f;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
    {
        dY = 1.0f;
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
    {
        dX = 1.0f;
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
    {
       dX = -1.0f;
    }

//    vec3 yAxis(transform->LocalToWorldDir(vec3(0.0, 1.0, 0.0)));
    vec3 yAxis(0.0, 1.0, 0.0);
    vec3 xAxis(transform->LocalToWorldDir(vec3(1.0, 0.0, 0.0)));
    xAxis.y = 0.0f;
    xAxis = normalize(xAxis);

    float avgDuration = 0.5f;
    float upStrAvgDuration = 0.8f;
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

    Debug::Log(std::to_string(speed));
    target = transform->GetWorldPosition() + target;

    transform->LookAt(target, upVector);
    //constant mouvement
    position += forward * deltaTime * speed;
    transform->SetWorldPosition(position);

}
