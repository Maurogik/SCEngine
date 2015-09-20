/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : SCE_GLDebug.cpp *******/
/**************************************/

#include "../headers/SCE_GLDebug.hpp"
#include "../headers/SCETools.hpp"

#ifdef SCE_DEBUG


#include <sstream>


void APIENTRY DebugCallbackAMD(
          GLuint id
        , GLenum category
        , GLenum severity
        , GLsizei length
        , const GLchar *message
        , GLvoid *userParam)
{
    FormatDebugOutput(category, category, id, severity, message);
}


void APIENTRY DebugCallback(
          GLenum source
        , GLenum type
        , GLuint id
        , GLenum severity
        , GLsizei length
        , const GLchar *message
        , GLvoid *userParam)
{
    FormatDebugOutput(source, type, id, severity, message);
}

std::string FormatDebugOutput(
          GLenum source
        , GLenum type
        , GLuint id
        , GLenum severity
        , const char* msg)
{

    std::stringstream stringStream;
    std::string sourceString;
    std::string typeString;
    std::string severityString;

    bool errorMustBreak = true;

    // The AMD variant of this extension provides a less detailed classification of the error,
    // which is why some arguments might be "Unknown".
    switch (source) {
        case GL_DEBUG_CATEGORY_API_ERROR_AMD:
        case GL_DEBUG_SOURCE_API: {
            sourceString = "API";
            break;
        }
        case GL_DEBUG_CATEGORY_APPLICATION_AMD:
        case GL_DEBUG_SOURCE_APPLICATION: {
            sourceString = "Application";
            break;
        }
        case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
            sourceString = "Window System";
            break;
        }
        case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
        case GL_DEBUG_SOURCE_SHADER_COMPILER: {
            sourceString = "Shader Compiler";
            break;
        }
        case GL_DEBUG_SOURCE_THIRD_PARTY: {
            sourceString = "Third Party";
            break;
        }
        case GL_DEBUG_CATEGORY_OTHER_AMD:
        case GL_DEBUG_SOURCE_OTHER: {
            sourceString = "Other";
            break;
        }
        default: {
            sourceString = "Unknown";
            break;
        }
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR: {
            typeString = "Error";
            break;
        }
        case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
            typeString = "Deprecated Behavior";
            break;
        }
        case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
            typeString = "Undefined Behavior";
            break;
        }
        case GL_DEBUG_TYPE_PORTABILITY_ARB: {
            typeString = "Portability";
            break;
        }
        case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
        case GL_DEBUG_TYPE_PERFORMANCE: {
            typeString = "Performance";
            break;
        }
        case GL_DEBUG_CATEGORY_OTHER_AMD:
        case GL_DEBUG_TYPE_OTHER: {
            typeString = "Other";
            break;
        }
        default: {
            typeString = "Unknown";
            break;
        }
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: {
            severityString = "High";
            break;
        }
        case GL_DEBUG_SEVERITY_MEDIUM: {
            severityString = "Medium";
            errorMustBreak = false;
            break;
        }
        case GL_DEBUG_SEVERITY_LOW: {
            severityString = "Low";
            errorMustBreak = false;
            break;
        }
        default: {
            severityString = "Unknown";
            errorMustBreak = false;
            break;
        }
    }

    stringStream << "[OpenGL Error]\n" << ">> " << msg << "\n";
    stringStream << "   [Source     = " << sourceString << "]\n";
    stringStream << "   [Type       = " << typeString << "]\n";;
    stringStream << "   [Severity   = " << severityString << "]\n";;
    stringStream << "   [ID         = " << id << "]\n";

    std::cout << stringStream.str() << std::endl;

    if(errorMustBreak){
         SCE::Debug::RaiseError("OpenGL Error raised");

    }

    return stringStream.str();
}


#endif

