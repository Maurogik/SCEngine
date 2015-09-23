/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/******FILE:SCEMetadataParser.hpp******/
/**************************************/
#ifndef SCE_METADATAPARSER_HPP
#define SCE_METADATAPARSER_HPP


#include "SCEDefines.hpp"
#include <map>

namespace SCE
{

namespace MetadataParser
{
    glm::vec3   StringToVec3(const std::string& str);
    glm::vec4   StringToVec4(const std::string& str);
    float       StringToFloat(const std::string& str);
    double      StringToDouble(const std::string& str);
    int         StringToInt(const std::string& str);
    std::map<std::string, std::string>
                GetLineData(const std::string& line);
    std::map<std::string, std::string>
                GetLineData(const std::string& line, char pairSeparator, char valueSeparator);
}

}

#endif
