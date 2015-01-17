/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETexture.cpp**********/
/**************************************/

#include "../headers/SCETexture.hpp"
#include "../external/SOIL/src/SOIL2.h"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCETools.hpp"

#include "external/rapidjson/document.h" // rapidjson's DOM-style API
#include "external/rapidjson/prettywriter.h" // for stringify JSON
#include "external/rapidjson/filestream.h" // wrapper of C stream for prettywriter as output

#include <iostream>
#include <fstream>


using namespace std;
using namespace SCE;

#define FORMAT_STR "Format"
#define DDS_STR "DDS"
#define UNCOMPRESSED_STR "Uncompressed"

#define MIPMAPS_STR "Mipmaps"
#define WRAPPING_STR "Wrapping"
#define CLAMP_STR "Clamp"
#define REPEAT_STR "Repeat"


SCETexture::SCETexture(const string &filename, const string &samplerName, GLuint shaderProgram)
    : mTextureId(0), mName(samplerName)
{
    mTextureId = loadTextureFromFile(filename, shaderProgram);
}

GLuint SCETexture::GetSamplerUniformId() const
{
    return mSamplerUniformId;
}

void SCETexture::BindTexture(GLuint textureUnit)
{
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(mSamplerUniformId, textureUnit);
}

GLuint SCETexture::GetTextureId() const
{
    return mTextureId;
}


GLuint SCETexture::loadTextureFromFile(const string &filename, GLuint shaderProgram)
{
    string fullTexturePath = RESSOURCE_PATH + filename;
    string metadataFile = fullTexturePath + TEXTURE_METADATA_SUFIX;


    //load texture metada

    rapidjson::Document root;
    string fileStr = "";
    ifstream fileStream(metadataFile.c_str(), ios::in);

    if(fileStream.is_open()){
        string currLine;
        while (getline(fileStream, currLine)) {
            fileStr += "\n" + currLine;
        }
        fileStream.close();

        //parse json string using rapidjson
        if (root.Parse<0>(fileStr.c_str()).HasParseError()){
            Debug::RaiseError("Error reading the texture file : " + filename);
        }

        //Access values
        Debug::Assert(root.IsObject(), "Malformated Json texture file\n");

        string name = root["Name"].GetString();
        SCEInternal::InternalMessage("Parsing texture data for : " + name);

        rapidjson::Value& textureRoot  = root["Texture"];

        Debug::Assert(textureRoot.IsObject(), "Texture value of Json texture file should be an object");

        //texture format
        string formatStr        = textureRoot[FORMAT_STR].GetString();
        bool mipmapsOn          = textureRoot[MIPMAPS_STR].GetBool();
        string wrappingModeStr  = textureRoot[WRAPPING_STR].GetString();

        SCETextureFormat format = formatFromString(formatStr);
        SCETextureWrap wrapMode = wrapModeFromString(wrappingModeStr);

        return loadTexture(filename, shaderProgram, format, wrapMode, mipmapsOn);

    } else {
        Debug::RaiseError("Failed to open file " + metadataFile);
        return 0;
    }
}

GLuint SCETexture::loadTexture(const string &filename, GLuint shaderProgram, SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn)
{
    string fullTexturePath = RESSOURCE_PATH + filename;
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

    mSamplerUniformId = glGetUniformLocation(shaderProgram, mName.c_str());

    return textureID;
}

SCETextureFormat SCETexture::formatFromString(const string &formatString)
{
    SCETextureFormat res = DDS_FORMAT;
    if(Tools::ToLowerCase(formatString) == Tools::ToLowerCase(DDS_STR)){
        res == DDS_FORMAT;
    } else if(Tools::ToLowerCase(formatString) == Tools::ToLowerCase(UNCOMPRESSED_STR)){
        res = UNCOMPRESSED_FORMAT;
    } else {
        Debug::RaiseError("Unknow texture format : " + formatString);
    }

    return res;
}

SCETextureWrap SCETexture::wrapModeFromString(const string &wrapString)
{
    SCETextureWrap res = CLAMP_WRAP;
    if(Tools::ToLowerCase(wrapString) == Tools::ToLowerCase(CLAMP_STR)){
        res == CLAMP_WRAP;
    } else if(Tools::ToLowerCase(wrapString) == Tools::ToLowerCase(REPEAT_STR)){
        res = REPEAT_WRAP;
    } else {
        Debug::RaiseError("Unknow texture wrap mode: " + wrapString);
    }

    return res;
}
