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
    uint FloatToColorRange(float val);
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
    float       Lamederp(float a, float b, float v);
    float       MapToRange(float fromMin, float fromMax, float toMin, float toMax, float val);
    void        SeedRandomGenerator(int val);
    float       RandRange(float min, float max);
    glm::vec3   GetRayPlaneIntersection(const glm::vec3& rayStart, const glm::vec3& rayDir,
                                        const vec3 &planeNormal, float planeDistToOrigin);
    glm::vec3   GetBarycentricCoord(const glm::vec3& point, const vec3 triangleVectices[]);
}

}

#endif
