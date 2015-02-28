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


SCETexture::SCETexture(const string &filename, const string &samplerName)
    : mTextureId(0), mName(samplerName), mWidth(-1), mHeight(-1)
{
    mTextureId = loadTextureFromFile(filename);
}

SCETexture::SCETexture(const string &samplerName, int width, int height, vec4 color, SCETextureFormat textureFormat, SCETextureWrap wrapMode, bool mipmapsOn)
    : mTextureId(0), mName(samplerName), mWidth(width), mHeight(height)
{
    mTextureId = createTexture(width, height, color, textureFormat, wrapMode, mipmapsOn);
}

SCETexture::~SCETexture()
{
    if(mTextureId != 0){
        glDeleteTextures(1, &mTextureId);
    }
}

void SCETexture::BindTexture(GLuint textureUnit, GLuint samplerUniformId)
{
    // Bind the texture to a texture Unit
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    // Set the sampler uniform to the texture unit
    glUniform1i(samplerUniformId, textureUnit);
}

GLuint SCETexture::GetTextureId() const
{
    return mTextureId;
}


GLuint SCETexture::loadTextureFromFile(const string &filename)
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
        Internal::Log("Parsing texture data for : " + name);

        rapidjson::Value& textureRoot  = root["Texture"];

        Debug::Assert(textureRoot.IsObject(), "Texture value of Json texture file should be an object");

        //texture format
        string formatStr        = textureRoot[FORMAT_STR].GetString();
        bool mipmapsOn          = textureRoot[MIPMAPS_STR].GetBool();
        string wrappingModeStr  = textureRoot[WRAPPING_STR].GetString();

        SCETextureFormat format = formatFromString(formatStr);
        SCETextureWrap wrapMode = wrapModeFromString(wrappingModeStr);

        return loadTexture(filename, format, wrapMode, mipmapsOn);

    } else {
        Debug::RaiseError("Failed to open file " + metadataFile);
        return 0;
    }
}

GLuint SCETexture::loadTexture(const string &filename, SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn)
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

GLuint SCETexture::createTexture(int width, int height, vec4 color, SCETextureFormat textureFormat, SCETextureWrap wrapMode, bool mipmapsOn)
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

    GLuint textureID = SOIL_create_OGL_texture(textureData, &mWidth, &mHeight, channelCount, 0, soilFlags);
    //GLuint textureID = SOIL_load_OGL_texture(fullTexturePath.c_str(), 0, 0, soilFlags);
    delete[](textureData);

    return textureID;
}

SCETextureFormat SCETexture::formatFromString(const string &formatString)
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

SCETextureWrap SCETexture::wrapModeFromString(const string &wrapString)
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

uint SCETexture::getSoilFlags(SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn)
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
