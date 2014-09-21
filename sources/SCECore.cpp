/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCECore.cpp *********/
/**************************************/

#include "../headers/SCECore.hpp"
#include "../headers/SCE_GLDebug.hpp"
#include "../headers/SCEInternal.hpp"

using namespace SCE;
using namespace std;

GLFWwindow * SCECore::s_window = 0l;


SCECore::SCECore()
{

}

SCECore::~SCECore()
{
    CleanUpEngine();
}

std::shared_ptr<SCECore> SCECore::InitEngine(const std::string &windowName)
{
    SCEInternal::InternalMessage("Initializing engine");

    // Initialise GLFW
    if( !glfwInit() )
    {
       Debug::RaiseError("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef SCE_DEBUG
    // Create a debug OpenGL context or tell your OpenGL library (GLFW, SDL) to do so.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    // Open a window and create its OpenGL context
    s_window = glfwCreateWindow( 1024, 768, windowName.c_str(), NULL, NULL);
    SCEInternal::InternalMessage("Window created");

    if( s_window == NULL ){
        glfwTerminate();
        Debug::RaiseError("Failed to open GLFW window.");
    }
    glfwMakeContextCurrent(s_window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        Debug::RaiseError("Failed to initialize GLEW.");
    }

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(s_window, GLFW_STICKY_KEYS, GL_TRUE);
    //glfwSetCursorPos(s_window, 1024/2, 768/2);

#ifdef SCE_DEBUG


    if(glDebugMessageCallbackAMD) {
        SCEInternal::InternalMessage("Linking GL debug with AMD callback");
        glDebugMessageCallbackAMD(DebugCallbackAMD, NULL);
    }
    else if(glDebugMessageCallbackARB) {
        SCEInternal::InternalMessage("Linking GL debug with ARB callback");
        glDebugMessageCallbackARB(DebugCallback, NULL);
    }
    else {
        SCEInternal::InternalMessage("Linking GL debug with standard callback");
        glDebugMessageCallback(DebugCallback, NULL);
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

#endif

    return shared_ptr<SCECore>(new SCECore());
}

void SCECore::RunEngine()
{

    do {
        Scene::Run();

        // Swap buffers
        glfwSwapBuffers(s_window);
        glfwPollEvents();

    } while(    glfwGetKey(s_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS
             && glfwWindowShouldClose(s_window) == 0 );


}

void SCECore::CleanUpEngine()
{
    SCEInternal::InternalMessage("Cleaning up engine");
    Scene::DestroyScene();
}

GLFWwindow *SCECore::GetWindow()
{
    return s_window;
}


