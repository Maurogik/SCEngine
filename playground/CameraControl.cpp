#include "CameraControl.hpp"
#include "../headers/SCECore.hpp"

using namespace SCE;
using namespace std;

CameraControl::CameraControl(SCE::SCEHandle<Container> container)
    : GameObject(container, "CamControl")
{
    double xMouse = 0.0;
    double yMouse = 0.0;
    GLFWwindow* window = SCECore::GetWindow();
    glfwGetCursorPos(window, &xMouse, &yMouse);

    xMouse /= SCECore::GetWindowWidth();
    yMouse /= SCECore::GetWindowHeight();

    lastMouseX = xMouse;
    lastMouseY = yMouse;
}

void CameraControl::Update()
{
    //move camera

    GLFWwindow* window = SCECore::GetWindow();
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    double xMouse = 0.0;
    double yMouse = 0.0;

    glfwGetCursorPos(window, &xMouse, &yMouse);

    xMouse /= SCECore::GetWindowWidth();
    yMouse /= SCECore::GetWindowHeight();

    float dX = xMouse - lastMouseX;
    float dY = yMouse - lastMouseY;

    lastMouseX = xMouse;
    lastMouseY = yMouse;

    float deltaTime = SCE::Time::DeltaTime();

    float xRotateSpeed = 4000.0f;
    float yRotateSpeed = 2000.0f;
    float speed = 30.0f;

    vec3 position = transform->GetWorldPosition();

    vec3 yAxis = transform->WorldToLocalDir(vec3(0.0, 1.0, 0.0));
    transform->RotateAroundAxis(yAxis, dX * xRotateSpeed * deltaTime);
    transform->RotateAroundAxis(vec3(1.0, 0.0, 0.0), dY * yRotateSpeed * deltaTime);

//    vec3 orientation = transform->GetLocalOrientation();
////    orientation.z = 0.0f;
//    orientation.y += dY * yRotateSpeed * deltaTime;
//    orientation.x += dX * xRotateSpeed * deltaTime;
//    transform->SetLocalOrientation(orientation);

    vec3 forward = transform->Forward();
    vec3 left = transform->Left();

    // Move forward
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        position += forward * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        position += -forward * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        position += -left * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        position += left * deltaTime * speed;
    }

    transform->SetWorldPosition(position);
}
