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
        GLuint      CreateShaderProgram(const std::string &shaderFileName);
        void        DeleteShaderProgram(GLuint shaderId);
        void        BindDefaultUniforms(GLuint shaderId, const glm::mat4& modelMatrix,
                                        const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void        BindRootPosition(GLuint shaderId, glm::vec3 const& rootPosition);
        void        UseShader(GLuint shaderProgram);
#ifdef SCE_DEBUG_ENGINE
        bool        ToggleDebugShader();
        void        ReloadShaders();
#endif
    }
}

#endif
