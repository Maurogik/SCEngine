/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETextures.cpp*********/
/**************************************/

#include "../headers/SCETextures.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"

#include "../external/SOIL/src/SOIL2.h"

#include "external/rapidjson/document.h" // rapidjson's DOM-style API
#include "external/rapidjson/prettywriter.h" // for stringify JSON
#include "external/rapidjson/filestream.h" // wrapper of C stream for prettywriter as output

#include <algorithm>
#include <iostream>
#include <fstream>

using namespace SCE;
using namespace std;



#define FORMAT_STR "Format"
#define DDS_STR "DDS"
#define UNCOMPRESSED_STR "UNCOMPRESSED"

#define MIPMAPS_STR "Mipmaps"
#define WRAPPING_STR "Wrapping"
#define CLAMP_STR "Clamp"
#define REPEAT_STR "Repeat"


SCETextures* SCETextures::s_instance = nullptr;


SCETextures::SCETextures()
    : mLoadedTextures(),
      mCreatedTextures()
{

}

SCETextures::~SCETextures()
{
    Internal::Log("Cleaning up texture system, will delete loaded textures");

    auto beginIt = begin(s_instance->mLoadedTextures);
    auto endIt = end(s_instance->mLoadedTextures);
    for(auto iterator = beginIt; iterator != endIt; iterator++) {
        Internal::Log("Deleting texture : " + iterator->first);
        glDeleteTextures(1, &(iterator->second));
    }

    for(GLuint texId : s_instance->mCreatedTextures)
    {
        glDeleteTextures(1, &texId);
    }
}

void SCETextures::Init()
{
    Debug::Assert(!s_instance, "An instance of the Texture system already exists");
    s_instance = new SCETextures();
}

void SCETextures::CleanUp()
{
    Debug::Assert(s_instance, "No Texture system instance found, Init the system before using it");
    delete s_instance;
}

GLuint SCETextures::CreateTexture(int width, int height,
                                  vec4 color,
                                  SCETextureFormat textureFormat, SCETextureWrap wrapMode,
                                  bool mipmapsOn)
{
    Debug::Assert(s_instance, "No Texture system instance found, Init the system before using it");

    GLuint texId = s_instance->createTexture(width, height,
                                     color,
                                     textureFormat, wrapMode,
                                     mipmapsOn);
    s_instance->mCreatedTextures.push_back(texId);

    return texId;
}

GLuint SCETextures::LoadTexture(const string& textureName)
{
    Debug::Assert(s_instance, "No Texture system instance found, Init the system before using it");

    //texture has already been loaded
    if(s_instance->mLoadedTextures.count(textureName) > 0)
    {
        Internal::Log("Texture " + textureName + " already loaded, using it directly");
        return s_instance->mLoadedTextures[textureName];
    }

    GLuint texId = s_instance->loadTextureFromMetadata(textureName);
    s_instance->mLoadedTextures[textureName] = texId;

    return texId;
}

void SCETextures::DeleteTexture(GLuint textureId)
{
    Debug::Assert(s_instance, "No Texture system instance found, Init the system before using it");

    auto itLoaded = find_if(begin(s_instance->mLoadedTextures),
                      end(s_instance->mLoadedTextures),
                      [&textureId](std::pair<string, GLuint> entry)
    { return entry.second == textureId; } );

    //texture id was found in loaded textures
    if(itLoaded != end(s_instance->mLoadedTextures))
    {
        Internal::Log("Delete texture : " + itLoaded->first);
        glDeleteTextures(1, &(itLoaded->second));
        s_instance->mLoadedTextures.erase(itLoaded);
    }

    auto itCreated = find(begin(s_instance->mCreatedTextures),
                   end(s_instance->mCreatedTextures),
                   textureId);

    //texture id was found in created textures
    if(itCreated != end(s_instance->mCreatedTextures))
    {
        glDeleteTextures(1, &textureId);
        s_instance->mCreatedTextures.erase(itCreated);
    }
}

void SCETextures::BindTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId)
{
    // Bind the texture to a texture Unit
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
    // Set the sampler uniform to the texture unit
    glUniform1i(samplerUniformId, textureUnit);
}



GLuint SCETextures::loadTextureFromMetadata(const string &filename)
{
    string fullTexturePath = RESSOURCE_PATH + filename;
    string metadataFile = fullTexturePath + TEXTURE_METADATA_SUFIX;

    //load texture metada

    rapidjson::Document root;
    string fileStr = "";
    ifstream fileStream(metadataFile.c_str(), ios::in);

    if(fileStream.is_open())
    {
        string currLine;
        while (getline(fileStream, currLine))
        {
            fileStr += "\n" + currLine;
        }
        fileStream.close();

        //parse json string using rapidjson
        if (root.Parse<0>(fileStr.c_str()).HasParseError())
        {
            Debug::RaiseError("Error reading the texture file : " + filename);
        }

        //Access values
        Debug::Assert(root.IsObject(), "Malformated Json texture file\n");

        string name = root["Name"].GetString();
        Internal::Log("Parsing texture data for : " + name);

        rapidjson::Value& textureRoot  = root["Texture"];

        Debug::Assert(textureRoot.IsObject(), "Texture value of Json texture file should be an object");

        //texture format
        string formatStr        = textureRoot[FORMAT_STR].GetString();
        bool mipmapsOn          = textureRoot[MIPMAPS_STR].GetBool();
        string wrappingModeStr  = textureRoot[WRAPPING_STR].GetString();

        SCETextureFormat format = formatFromString(formatStr);
        SCETextureWrap wrapMode = wrapModeFromString(wrappingModeStr);

        return loadTexture(name, format, wrapMode, mipmapsOn);

    }
    else
    {
        Debug::RaiseError("Failed to open file " + metadataFile);
        return 0;
    }
}

GLuint SCETextures::loadTexture(const string &filename,
                                SCETextureFormat format, SCETextureWrap wrapMode,
                                bool mipmapsOn)
{
    string fullTexturePath = RESSOURCE_PATH + filename;
    uint soilFlags = getSoilFlags(format, wrapMode, mipmapsOn);

    Internal::Log("TODO : Find a way to get the width and height of the loaded texture");

    //create a new openGL texture
    //force_channels : 0 = channels auto
    //reuse texture : 0 = create new texture
    GLuint textureID = SOIL_load_OGL_texture(fullTexturePath.c_str(), 0, 0, soilFlags);

//    GLuint textureID = 0;
//    int width, height;
//    SCEInternal::InternalMessage("try to load texture : " + fullTexturePath);
//    unsigned char* image = SOIL_load_image(fullTexturePath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);

//    glGenTextures(1, &textureID);
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
//                  GL_UNSIGNED_BYTE, image);

//    SOIL_free_image_data(image);

    return textureID;
}

GLuint SCETextures::createTexture(int width, int height,
                                  vec4 color,
                                  SCETextureFormat textureFormat, SCETextureWrap wrapMode,
                                  bool mipmapsOn)
{
    int channelCount = 4;
    uint soilFlags = getSoilFlags(textureFormat, wrapMode, mipmapsOn);
    unsigned char *textureData = new unsigned char [width * height * channelCount];
    int xstart, ystart;
    for(int x = 0; x < width; ++x){
        xstart = x * height * channelCount;
        for(int y = 0; y < height; ++y){
            ystart = y * channelCount;
            textureData[xstart + ystart + 0] = Tools::floatToColorRange(color.x);
            textureData[xstart + ystart + 1] = Tools::floatToColorRange(color.y);
            textureData[xstart + ystart + 2] = Tools::floatToColorRange(color.z);
            textureData[xstart + ystart + 3] = Tools::floatToColorRange(color.w);
        }
    }

    GLuint textureID = SOIL_create_OGL_texture(textureData, &width, &height, channelCount, 0, soilFlags);
    //GLuint textureID = SOIL_load_OGL_texture(fullTexturePath.c_str(), 0, 0, soilFlags);
    delete[](textureData);

    return textureID;
}

SCETextureFormat SCETextures::formatFromString(const string &formatString)
{
    SCETextureFormat res = DDS_FORMAT;
    if(Tools::ToLowerCase(formatString) == Tools::ToLowerCase(DDS_STR)){
        //res == DDS_FORMAT;
    } else if(Tools::ToLowerCase(formatString) == Tools::ToLowerCase(UNCOMPRESSED_STR)){
        res = UNCOMPRESSED_FORMAT;
    } else {
        Debug::RaiseError("Unknow texture format : " + formatString);
    }

    return res;
}

SCETextureWrap SCETextures::wrapModeFromString(const string &wrapString)
{
    SCETextureWrap res = CLAMP_WRAP;
    if(Tools::ToLowerCase(wrapString) == Tools::ToLowerCase(CLAMP_STR)){
        //res == CLAMP_WRAP;
    } else if(Tools::ToLowerCase(wrapString) == Tools::ToLowerCase(REPEAT_STR)){
        res = REPEAT_WRAP;
    } else {
        Debug::RaiseError("Unknow texture wrap mode: " + wrapString);
    }

    return res;
}

uint SCETextures::getSoilFlags(SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn)
{
    uint soilFlags = 0;

    switch(format) {
    case DDS_FORMAT :
        soilFlags |= SOIL_FLAG_COMPRESS_TO_DXT;
        break;
    case UNCOMPRESSED_FORMAT :
        //if not specified, Soil use uncompressed texture
        break;
    default ://DDS_FORMAT is the default
        soilFlags |= SOIL_FLAG_COMPRESS_TO_DXT;
        break;
    }

    switch(wrapMode){
    case REPEAT_WRAP :
        soilFlags |= SOIL_FLAG_TEXTURE_REPEATS;
        break;
    case CLAMP_WRAP :
        //if not specified, CLAMP is used
        break;
    }

    if(mipmapsOn) {
        soilFlags |= SOIL_FLAG_GL_MIPMAPS;
    }

    return soilFlags;
}
