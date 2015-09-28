/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : SCE_GLDebug.hpp *******/
/**************************************/
#ifndef SCE_GL_DEBUG_HPP
#define SCE_GL_DEBUG_HPP


#include "SCEDefines.hpp"


#ifdef SCE_DEBUG

// Include GLFW
#include <glfw3.h>



void APIENTRY DebugCallbackAMD(
          GLuint id
        , GLenum category
        , GLenum severity
        , GLsizei length
        , const GLchar* message
        , GLvoid* userParam);

void APIENTRY DebugCallback(
          GLenum source
        , GLenum type
        , GLuint id
        , GLenum severity
        , GLsizei length
        , const GLchar* message
        , GLvoid* userParam);


std::string FormatDebugOutput(
          GLenum source
        , GLenum type
        , GLuint id
        , GLenum severity
        , const char* msg);


#endif

#endif


