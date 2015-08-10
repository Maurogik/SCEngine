/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.cpp*********/
/**************************************/

#include "../headers/SCELighting.hpp"
#include "../headers/SCETools.hpp"

using namespace SCE;

#define LIGHT_SHADER_NAME "DeferredLighting"

GLuint SCELighting::s_DefaultLightShader = (GLuint) -1;

void SCELighting::StartLightPass()
{
    glUseProgram(s_DefaultLightShader);
}

void SCELighting::initLightShader()
{
    if(s_DefaultLightShader == (GLuint) -1){
        s_DefaultLightShader = ShaderTools::CompileShader(LIGHT_SHADER_NAME);
    }
}

void SCELighting::Init()
{
    initLightShader();
}

void SCELighting::CleanUp()
{
    //unload shader early
    glDeleteProgram(s_DefaultLightShader);
    s_DefaultLightShader = (GLuint) -1;
}

GLuint SCELighting::GetLightShader()
{
    return s_DefaultLightShader;
}
