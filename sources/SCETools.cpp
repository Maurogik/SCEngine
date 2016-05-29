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
    #else
        LogError(errorMsg);
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

    uint FloatToColorRange(float val)
    {
        return (int)(val * 255.0f);
    }
}

namespace Math
{
    float MapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = glm::max(fromMin, (glm::min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return glm::mix(toMin, toMax, val);
    }

    void SeedRandomGenerator(int val)
    {
        srand(val);
        int r = rand();
        srand(r);
        r = rand();
        srand(r);
        r = rand();
        srand(r);
    }

    float RandRange(float min, float max)
    {
        return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
    }

    vec3 GetRayPlaneIntersection(const vec3 &rayStart, const vec3 &rayDir,
                                 const vec3 &planeNormal, float planeDistToOrigin)
    {
        glm::vec4 plane(planeNormal, planeDistToOrigin);
        glm::vec4 start(rayStart, 1.0);
        glm::vec4 dir(rayDir, 0.0);

        float planeDotDir = glm::dot(plane, dir);
        if(planeDotDir != 0.0f)
        {
            float distOnRay = -glm::dot(plane, start)/planeDotDir;
            return rayStart + distOnRay*rayDir;
        }
        else
        {
            Debug::LogError("Intersection between plane an ray coul not be found !!");
            return vec3(0.0f);
        }
    }

    vec3 GetBarycentricCoord(const vec3 &point, const vec3 triangleVectices[3])
    {
        glm::vec3 r = point - triangleVectices[0];
        glm::vec3 q1 = triangleVectices[1] - triangleVectices[0];
        glm::vec3 q2 = triangleVectices[2] - triangleVectices[0];

        glm::vec2 dotVec (glm::dot(r, q1), glm::dot(r, q2));
        glm::mat2 m(
            glm::dot(q1, q1), glm::dot(q1, q2),
            glm::dot(q1, q2), glm::dot(q2, q2)
        );

        glm::vec2 w1w2 = m*dotVec;
        return vec3(1.0f - w1w2.x - w1w2.y, w1w2.x, w1w2.y);
    }

    void CombineAABB(const vec3 &centerA, const vec3 &dimA,
                     const vec3 &centerB, const vec3 &dimB,
                     vec3 &centerRes, vec3 &dimRes)
    {
        std::vector<glm::vec3> points;
        points.reserve(4);
        points.push_back(centerA + dimA);
        points.push_back(centerA - dimA);
        points.push_back(centerB + dimB);
        points.push_back(centerB - dimB);

        GetAABBForPoints(points, centerRes, dimRes);
    }

    void GetAABBForPoints(const std::vector<vec3> &positions, vec3 &center, vec3 &dimentions)
    {
        glm::vec3 minValues = glm::vec3(0.0f);
        glm::vec3 maxValues = glm::vec3(0.0f);

        for(uint i = 0; i < positions.size(); ++i)
        {
            if(minValues.x > positions[i].x)
            {
                minValues.x = positions[i].x;
            }

            if(minValues.y > positions[i].y)
            {
                minValues.y = positions[i].y;
            }

            if(minValues.z > positions[i].z)
            {
                minValues.z = positions[i].z;
            }

            if(maxValues.x < positions[i].x)
            {
                maxValues.x = positions[i].x;
            }

            if(maxValues.y < positions[i].y)
            {
                maxValues.y = positions[i].y;
            }

            if(maxValues.z < positions[i].z)
            {
                maxValues.z = positions[i].z;
            }
        }

        center = (maxValues + minValues)*0.5f;
        dimentions = (maxValues - minValues)*0.5f;
    }

}

}









