/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEShaders.hpp**********/
/**************************************/
#ifndef SCE_SHADERS_HPP
#define SCE_SHADERS_HPP

#include "../headers/SCEDefines.hpp"

namespace SCE
{
    namespace ShaderUtils
    {
        GLuint       CreateShaderProgram(const std::string &shaderFileName);
        void         DeleteShaderProgram(GLuint shaderId);
        void         BindDefaultUniforms(GLuint shaderId);
    }
}

#endif
