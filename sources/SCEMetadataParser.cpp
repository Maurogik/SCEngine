/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/******FILE:SCEMetadataParser.cpp******/
/**************************************/

#include "../headers/SCEMetadataParser.hpp"
#include "../headers/SCETools.hpp"

#include <vector>

using namespace std;

#define PAIR_SEP ':'
#define VALUE_SEP ';'

namespace SCE
{

namespace MetadataParser
{
    vec3 StringToVec3(const string &str)
    {
        float v1 = -666.0f, v2 = -666.0f, v3 = -666.0f;
        int nbRead = sscanf(str.c_str(), "(%f,%f,%f)", &v1, &v2, &v3);
        Debug::Assert(nbRead == 3, "Could not read vec3 from string");
        return glm::vec3(v1, v2, v3);
    }


    vec4 StringToVec4(const string &str)
    {
        float v1 = -666.0f, v2 = -666.0f, v3 = -666.0f, v4 = -666.0f;
        int nbRead = sscanf(str.c_str(), "(%f,%f,%f,%f)", &v1, &v2, &v3, &v4);
        Debug::Assert(nbRead == 4, "Could not read vec4 from string");
        return glm::vec4(v1, v2, v3, v4);
    }


    float StringToFloat(const string &str)
    {
        return (float)atof(str.c_str());
    }


    double StringToDouble(const string &str)
    {
        return atof(str.c_str());
    }


    int StringToInt(const string &str)
    {
        return (int)atoi(str.c_str());
    }

    map<string, string> GetLineData(const string& line)
    {
        return GetLineData(line, PAIR_SEP, VALUE_SEP);
    }

    map<string, string> GetLineData(const string& line, char pairSeparator, char valueSeparator)
    {
        map<string, string> lineData;
        string remainingLine = line;

        vector<string> linePairs;
        size_t valueSepPos = 0;
        while(valueSepPos != string::npos)
        {
            valueSepPos = remainingLine.find_first_of(valueSeparator);
            linePairs.push_back(remainingLine.substr(0, valueSepPos));
            if(valueSepPos != string::npos)
            {
                remainingLine = remainingLine.substr(valueSepPos + 1);
            }
        }

        for(string pairString : linePairs)
        {
            int sepInd = pairString.find_first_of(pairSeparator);
            string key = pairString.substr(0, sepInd);
            string value = pairString.substr(sepInd + 1);

            if(key[0] == ' ')
            {
                key = key.substr(1);
            }

            if(value[0] == ' ')
            {
                value = value.substr(1);
            }

            lineData[key] = value;
        }

        return lineData;
    }
}

}
