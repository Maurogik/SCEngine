/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCECore.hpp *********/
/**************************************/
#ifndef SCE_CORE_HPP
#define SCE_CORE_HPP

#include "SCEDefines.hpp"
#include "SCE.hpp"

class GLFWwindow;

namespace SCE {

    class SCECore {

    public :

                                            SCECore() = default;
                                            ~SCECore();

        void                                InitEngine(const std::string &windowName);
        void                                RunEngine();

        static GLFWwindow*                  GetWindow();
        static int                          GetWindowWidth();
        static int                          GetWindowHeight();
        static void                         UpdateWindow();

    private :

        void                                CleanUpEngine();

        static int          s_windowWidth;
        static int          s_windowHeight;
        static GLFWwindow*  s_window;
    };

}

#endif
