/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/
#ifndef SCE_TOOLS_HPP
#define SCE_TOOLS_HPP

// Include GLFW
#include "SCEDefines.hpp"
#include <iostream>
#include <stdio.h>

namespace SCE {

namespace Parser {

    glm::vec3   StringToVec3(const std::string& str);
    glm::vec4   StringToVec4(const std::string& str);
    float       StringToFloat(const std::string& str);
    double      StringToDouble(const std::string& str);
    int         StringToInt(const std::string& str);
}

namespace Tools {

    int         HashFromString(const std::string& str);
    std::string ToLowerCase(const std::string &str);
    int         floatToColorRange(float val);
}

namespace Debug {

    void        Assert(bool condition, const std::string& message);
    void        RaiseError(const std::string& errorMsg);
    void        Log(const std::string& message);
    void        LogError(const std::string& message);

}

namespace ShaderTools {

    GLuint      CompileShader(const std::string& shaderFileName);

}


}

#endif
