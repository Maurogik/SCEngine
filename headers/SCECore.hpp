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

                                            SCECore() = default;
                                            ~SCECore();

        void                                InitEngine(const std::string &windowName);
        void                                RunEngine();

        static GLFWwindow*                  GetWindow();

    private :

        void                                CleanUpEngine();

        static GLFWwindow* s_window;
    };

}

#endif
