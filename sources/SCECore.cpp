/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCECore.cpp *********/
/**************************************/

#include "../headers/SCECore.hpp"
#include "../headers/SCE_GLDebug.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCELighting.hpp"

using namespace SCE;
using namespace std;

GLFWwindow *    SCECore::s_window       = nullptr;
int             SCECore::s_windowWidth  = 0;
int             SCECore::s_windowHeight = 0;

SCECore::~SCECore()
{
    CleanUpEngine();
}

void SCECore::InitEngine(const std::string &windowName)
{
    Internal::Log("Initializing engine");

    // Initialise GLFW
    if( !glfwInit() )
    {
       Debug::RaiseError("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    //set framebuffer to suppoer 32 depth bits and 8 stencil bits
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);


#ifdef SCE_DEBUG
    // Create a debug OpenGL context or tell your OpenGL library (GLFW, SDL) to do so.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    // Open a window and create its OpenGL context
    s_window = glfwCreateWindow( 1024, 768, windowName.c_str(), NULL, NULL);
    Internal::Log("Window created");

    if( s_window == NULL ){
        glfwTerminate();
        Debug::RaiseError("Failed to open GLFW window.");
    }
    glfwMakeContextCurrent(s_window);
    glewExperimental=true; // Needed in core profile

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        Debug::RaiseError("Failed to initialize GLEW.");
    }

    UpdateWindow();

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(s_window, GLFW_STICKY_KEYS, GL_TRUE);
    //glfwSetCursorPos(s_window, 1024/2, 768/2);

#ifdef SCE_DEBUG

    if(glDebugMessageCallbackAMD) {
        Internal::Log("Linking GL debug with AMD callback");
        glDebugMessageCallbackAMD(DebugCallbackAMD, NULL);
    }
    else if(glDebugMessageCallbackARB) {
        Internal::Log("Linking GL debug with ARB callback");
        glDebugMessageCallbackARB(DebugCallback, NULL);
    }
    else {
        Internal::Log("Linking GL debug with standard callback");
        glDebugMessageCallback(DebugCallback, NULL);
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

#endif

    //Init Engine subcomponents in order
    SCETime::Init();
    SCELighting::Init();
}

void SCECore::RunEngine()
{

    do {
        SCETime::Update();
        SCEScene::Run();

        // Swap buffers
        glfwSwapBuffers(s_window);
        glfwPollEvents();

    } while( glfwGetKey(s_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS
             && glfwWindowShouldClose(s_window) == 0 );

}

void SCECore::CleanUpEngine()
{
    Internal::Log("Cleaning up engine");
    SCEScene::DestroyScene();

    SCELighting::CleanUp();
    SCETime::CleanUp();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

GLFWwindow *SCECore::GetWindow()
{
    return s_window;
}

int SCECore::GetWindowWidth()
{
    return s_windowWidth;
}

int SCECore::GetWindowHeight()
{
    return s_windowHeight;
}

void SCECore::UpdateWindow()
{
    int width, height;
    glfwGetWindowSize(s_window, &width, &height);
    s_windowWidth = width;
    s_windowHeight = height;
}


