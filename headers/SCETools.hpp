/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/
#ifndef SCE_TOOLS_HPP
#define SCE_TOOLS_HPP

// Include GLFW
#include "SCEDefines.hpp"
#include <stdio.h>

namespace SCE {

namespace Tools
{
    int         HashFromString(const std::string& str);
    std::string ToLowerCase(const std::string &str);
    uint floatToColorRange(float val);
}

namespace Debug
{
    void        Assert(bool condition, const std::string& message);
    void        RaiseError(const std::string& errorMsg);
    void        Log(const std::string& message);
    void        LogError(const std::string& message);
}

namespace Math
{
    float lerp(float a, float b, float v);
    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val);
    void seedRandomGenerator(int val);
    float randRange(float min, float max);
}

}

#endif
