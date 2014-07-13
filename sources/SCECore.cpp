/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCECore.cpp *********/
/**************************************/

#include "../headers/SCECore.hpp"
#include "../headers/SCEDebug.hpp"

using namespace SCE;

GLFWwindow * SCECore::s_window = 0l;

void SCECore::InitEngine(const std::string &windowName)
{
    SCE_DEBUG_LOG("Initializing engine");

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return;
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
    SCE_DEBUG_LOG("Window created");

    if( s_window == NULL ){
        fprintf( stderr, "Failed to open GLFW window.\n" );
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(s_window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(s_window, GLFW_STICKY_KEYS, GL_TRUE);
    //glfwSetCursorPos(s_window, 1024/2, 768/2);

#ifdef SCE_DEBUG


    if(glDebugMessageCallbackAMD) {
        SCE_DEBUG_LOG("Linking GL debug with AMD callback");
        glDebugMessageCallbackAMD(DebugCallbackAMD, NULL);
    }
    else if(glDebugMessageCallbackARB) {
        SCE_DEBUG_LOG("Linking GL debug with ARB callback");
        glDebugMessageCallbackARB(DebugCallback, NULL);
    }
    else {
        SCE_DEBUG_LOG("Linking GL debug with standard callback");
        glDebugMessageCallback(DebugCallback, NULL);
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

#endif

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
    SCE_DEBUG_LOG("Cleaning up engine");
    Scene::DestroyScene();
}

GLFWwindow *SCECore::GetWindow()
{
    return s_window;
}
