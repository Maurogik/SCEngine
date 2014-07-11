/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/
#ifndef SCE_TOOLS_HPP
#define SCE_TOOLS_HPP

#include <string>
// Include GLFW
#include "SCEDefines.hpp"


namespace SCE {

    glm::vec3    StringToVec3(const std::string& str);
    glm::vec4    StringToVec4(const std::string& str);
    float        StringToFloat(const std::string& str);
    double       StringToDouble(const std::string& str);
    int          StringToInt(const std::string& str);

}

#endif
