/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/

#include "../headers/SCETools.hpp"
#include <stdlib.h>

using namespace std;

namespace SCE {

namespace Debug
{
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
        LogError(errorMsg);
        abort();
    #endif
    }

    void Log(const string &message)
    {
        cout << message << endl;
    }

    void LogError(const string &message)
    {
        cerr << "[ERROR] " << message << endl;
    }
}

namespace Tools
{
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

    uint floatToColorRange(float val)
    {
        return (int)(val * 255.0f);
    }
}

namespace Math
{
    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = glm::max(fromMin, (glm::min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return lerp(toMin, toMax, val);
    }

    float lerp(float a, float b, float v)
    {
        return a + (b - a) * v;
    }

    void seedRandomGenerator(float val)
    {
        srand(val);
    }

    float randRange(float min, float max)
    {
        return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
    }
}

}









