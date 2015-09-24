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

    lastMouseX = xMouse;
    lastMouseY = yMouse;
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

    float dX = xMouse - 0.5f;// - lastMouseX;
    float dY = yMouse - 0.5f;// - lastMouseY;

    lastMouseX = xMouse;
    lastMouseY = yMouse;

    float deltaTime = SCE::Time::DeltaTime();

//    float xRotateSpeed = 4000.0f;
//    float yRotateSpeed = 2000.0f;

    float xRotateSpeed = 50.0f;
    float yRotateSpeed = 50.0f;

    float speed = 30.0f;

    vec3 position = transform->GetWorldPosition();
    vec3 forward = transform->Forward();
    //constant mouvement
    position += forward * deltaTime * speed;

    dX = 0.0f;
    dY = 0.0f;
    // Move forward
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS)
    {
        dY = 1.0f;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS ||
            glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
    {
        dY = -1.0f;
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


//    vec3 yAxis = transform->WorldToLocalDir(vec3(0.0, 1.0, 0.0));
//    vec3 xAxis = vec3(1.0, 0.0, 0.0);//transform->WorldToLocalDir(vec3(1.0, 0.0, 0.0));
//    vec3 zAxis = transform->WorldToLocalDir(vec3(0.0, 0.0, 1.0));


    vec3 yAxis(0.0, 1.0, 0.0);
    vec3 xAxis(transform->LocalToWorldDir(vec3(1.0, 0.0, 0.0)));
    xAxis.y = 0.0f;
    xAxis = normalize(xAxis);

    vec3 target = dX * deltaTime * xAxis + dY * deltaTime * yAxis + transform->Forward();
    target.y = glm::clamp(target.y, -0.75f, 0.75f);
    target = transform->GetWorldPosition() + target;

    vec3 upVector = normalize(dX * deltaTime * xAxis * 10.0f + vec3(0.0, 1.0, 0.0));

//    vec3 yAxis(0.0, 1.0, 0.0);
//    vec3 xAxis(1.0, 0.0, 0.0);
//    vec3 zAxis(0.0, 0.0, 1.0);

//    transform->RotateAroundAxis(yAxis, dX * xRotateSpeed * deltaTime);
//    transform->RotateAroundAxis(xAxis, dY * yRotateSpeed * deltaTime);

//    vec3 orientation = transform->GetLocalOrientation();
//    orientation.z = -dX * 40.0f;
//    transform->SetLocalOrientation(orientation);

    transform->LookAt(target, upVector);
    transform->SetWorldPosition(position);

}
