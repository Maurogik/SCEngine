/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEShaders.hpp**********/
/**************************************/
#ifndef SCE_SHADERS_HPP
#define SCE_SHADERS_HPP

#include "SCEDefines.hpp"
#include <map>

namespace SCE
{

    class SCEShaders
    {

    public :

        static void         Init();
        static void         CleanUp();
        static GLuint       CreateShaderProgram(const std::string &shaderFileName);
        static void         DeleteShaderProgram(GLuint shaderId);
        static void         BindDefaultUniforms(GLuint shaderId);

    private :

        enum ShaderType
        {
            FRAGMENT_SHADER = 0,
            VERTEX_SHADER,
            TESSELATION_EVALUATION_SHADER,
            TESSELATION_CONTROL_SHADER,
            SHADER_TYPE_COUNT
        };

        struct DefaultUniforms
        {
            GLint screenSizeUniform;
        };

        static SCEShaders*                  s_instance;

        std::map<std::string, GLuint>       mCompiledShaderPrograms;
        std::map<GLuint, DefaultUniforms>   mDefaultUniforms;

        SCEShaders();
        ~SCEShaders();
    };
}

#endif
