/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCEInput.cpp***********/
/**************************************/

#include "../headers/SCEInput.hpp"
#include <glfw3.h>

namespace SCE
{
namespace Input
{
#define KEY_COUNT (GLFW_KEY_LAST + 1)
    namespace
    {
        ui16 keyPrevStates[KEY_COUNT] = { 0 }; //zero initialize whole array
        ui16 keyStates[KEY_COUNT] = { 0 }; //zero initialize whole array
    }

    void UpdateKeyStates(GLFWwindow *window)
    {
        if(window)
        {
            for(int i = 0; i < KEY_COUNT; ++i)
            {
                keyPrevStates[i] = keyStates[i];
                keyStates[i] = glfwGetKey(window, i);
            }
        }
    }

    KeyAction GetKeyAction(int key)
    {
        if(keyStates[key] == GLFW_PRESS && keyPrevStates[key] == GLFW_PRESS)
        {
            return KeyAction::Hold;
        }
        if(keyStates[key] == GLFW_PRESS && keyPrevStates[key] == GLFW_RELEASE)
        {
            return KeyAction::Press;
        }
        if(keyStates[key] == GLFW_RELEASE && keyPrevStates[key] == GLFW_PRESS)
        {
            return KeyAction::Release;
        }

        return KeyAction::None;
    }
}
}
