/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/

#include "../headers/SCETools.hpp"


using namespace std;
using namespace glm;


using namespace SCE;

namespace SCE {


vec3 StringToVec3(const string &str)
{
    float v1 = -666.0f, v2 = -666.0f, v3 = -666.0f;
    sscanf(str.c_str(), "(%f,%f,%f)", &v1, &v2, &v3);
    return vec3(v1, v2, v3);
}


vec4 StringToVec4(const string &str)
{
    float v1 = -666.0f, v2 = -666.0f, v3 = -666.0f, v4 = -666.0f;
    sscanf(str.c_str(), "(%f,%f,%f,%f)", &v1, &v2, &v3, &v4);
    return vec4(v1, v2, v3, v4);
}


float StringToFloat(const string &str)
{
    return (float)atof(str.c_str());
}


double StringToDouble(const string &str)
{
    return (double)atof(str.c_str());
}


int StringToInt(const string &str)
{
    return (int)atoi(str.c_str());
}


}
