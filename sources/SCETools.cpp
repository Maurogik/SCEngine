/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/

#include "../headers/SCETools.hpp"

#include <assert.h>

using namespace std;
using namespace glm;


namespace SCE {

namespace Debug {

    void Assert(bool condition, const string &message)
    {
    #ifdef SCE_DEBUG
        if(!condition){
            RaiseError("Assertion failed : " + message);
        }
    #endif
    }

    void RaiseError(const string &errorMsg)
    {
    #ifdef SCE_DEBUG
        PrintError(errorMsg);
        abort();
    #endif
    }

    void PrintMessage(const string &message)
    {
        cout << message << endl;
    }

    void PrintError(const string &message)
    {
        cerr << "[ERROR] " << message << endl;
    }

}


namespace Parser {

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


namespace Tools {

    int HashFromString(const string &str)
    {
        std::hash<std::string> hash_func;
        return hash_func(str);
    }

    string ToLowerCase(const string &str)
    {
        string res = str;
        std::transform(begin(res), end(res), begin(res), ::tolower);
        return res;
    }

}

}









