/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCEInput.hpp***********/
/**************************************/
#ifndef SCE_SCEINPUT_HPP
#define SCE_SCEINPUT_HPP

#include "SCEDefines.hpp"

//class GLFWwindow;
//include glfw for now
#include <glfw3.h>

namespace SCE
{
    namespace Input
    {
        enum KeyAction
        {
            Press = 0,
            Release,
            Hold,
            None,
            Count
        };

        void        Initialize();
        void        UpdateKeyStates(GLFWwindow *window);
        KeyAction   GetKeyAction(int key);
    }
}

#endif
