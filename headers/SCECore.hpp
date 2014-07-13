/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCECore.hpp *********/
/**************************************/
#ifndef SCE_CORE_HPP
#define SCE_CORE_HPP

#include "SCEDefines.hpp"
#include "SCE.hpp"

// Include GLFW
#include <glfw3.h>


namespace SCE {

    class SCECore {

    public :
        static void InitEngine(const std::string &windowName);
        static void RunEngine();
        static void CleanUpEngine();
        static GLFWwindow* GetWindow();

    private :
        static GLFWwindow* s_window;
    };

}

#endif
