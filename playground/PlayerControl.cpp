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
    SCE::DebugText::Print("Frame : " + std::to_string(deltaTime*1000.0f) + " ms", printColor);

//    float xRotateSpeed = 0.015f;
//    float yRotateSpeed = 0.015f;
    float xRotateSpeed = 1.0f;
    float yRotateSpeed = 1.0f;
    float zRotateSpeed = 1.5f;
    float speed = 30.0f;
    float diveSpeedIncrease = 50.0f;
    float climbSlowdown = 10.0f;

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
       speed = 500.0f;
#else
       speed = 500.0f;
#endif
    }
    else if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS)
    {
#ifdef SCE_DEBUG
       speed = 1.0f;
#else
//       speed = 5.0f;
        speed = 5.0f;
        lastSpeed *= 0.8f;
#endif
    }


//    vec3 yAxis(transform->LocalToWorldDir(vec3(0.0, 1.0, 0.0)));
    vec3 yAxis(0.0, 1.0, 0.0);
    vec3 xAxis(transform->LocalToWorldDir(vec3(1.0, 0.0, 0.0)));
    xAxis.y = 0.0f;
    xAxis = normalize(xAxis);

    float avgDuration = 0.4f;
    float upStrAvgDuration = 0.3f;
    averageDx = (averageDx*avgDuration + dX*deltaTime)/(avgDuration + deltaTime);
    averageDy = (averageDy*avgDuration + dY*deltaTime)/(avgDuration + deltaTime);

    dX = averageDx*xRotateSpeed*deltaTime;
    dY = averageDy*yRotateSpeed*deltaTime;

    vec3 target = dX*xAxis + dY*yAxis + transform->Forward();

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

    SCE::DebugText::Print("Mvt speed : " + std::to_string(speed));

    float turnStrength = glm::clamp(40.0f/speed, 0.1f, 2.0f);
    target = transform->GetWorldPosition() + target * turnStrength;

    transform->LookAt(target, upVector);
    //constant mouvement
    vec3 position = transform->GetWorldPosition();
    position += transform->Forward()*deltaTime*speed;
    transform->SetWorldPosition(position);

}
