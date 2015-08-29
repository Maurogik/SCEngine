/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.cpp ********/
/**************************************/

#include "../headers/Material.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCEScene.hpp"
#include "../headers/SCETextures.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/Transform.hpp"

#include "external/rapidjson/document.h" // rapidjson's DOM-style API
#include "external/rapidjson/prettywriter.h" // for stringify JSON
#include "external/rapidjson/filestream.h" // wrapper of C stream for prettywriter as output

#include <iostream>
#include <fstream>


using namespace SCE;
using namespace std;

#define NO_SHADER_ID 999999


Material::Material(SCEHandle<Container> &container, const string &filename, const string &typeName)
    : Component(container, "Material::" + typeName) ,
      mMaterialName("Material not loaded yet"),
      mShaderProgramId(NO_SHADER_ID),
      mUniforms()
{
    LoadMaterial(filename);
}


Material::~Material()
{
    //free each allocated structures
    for(auto it = begin(mUniforms); it != end(mUniforms); ++it)
    {
        uniform_data data = it->second;
        if(data.type == UNIFORM_FLOAT)
        {
            float* fData = (float*) data.data;
            delete(fData);
        }
        else if (data.type == UNIFORM_VEC3)
        {
            glm::vec3* fData = (glm::vec3*) data.data;
            delete(fData);
        }
        else if (data.type == UNIFORM_VEC4)
        {
            glm::vec4* fData = (glm::vec4*) data.data;
            delete(fData);
        }
        else if (data.type == UNIFORM_TEXTURE2D)
        {
            GLuint* fData = (GLuint*) data.data;
            SCETextures::DeleteTexture(*fData);
            delete(fData);
        }
    }

    //unload shader
    SCEShaders::DeleteShaderProgram(mShaderProgramId);
}

void Material::LoadMaterial(const string &filename)
{
    string fullPath = RESSOURCE_PATH + filename + MATERIAL_SUFIX;

    rapidjson::Document root;
    ifstream materialFileStream(fullPath.c_str(), ios::in);

    if(materialFileStream.is_open())
    {
        string currLine;
        string matFileStr = "";
        //read whole file in a string
        while (getline(materialFileStream, currLine))
        {
            matFileStr += "\n" + currLine;
        }
        materialFileStream.close();

        if (root.Parse<0>(matFileStr.c_str()).HasParseError())
        {
            Debug::RaiseError("Error reading the material file : " + filename + "\n error : " + root.Parse<0>(matFileStr.c_str()).GetParseError());
        }

        //Access values
        Debug::Assert(root.IsObject(), "Malformated Json material file\n");

        mMaterialName = root["Name"].GetString();
        Internal::Log("Parsing material : " + mMaterialName);

        rapidjson::Value& materialRoot = root["Material"];
        rapidjson::Value& materialData = materialRoot["Data"];
        string shaderName   = root["Shader"].GetString();

        mShaderProgramId = loadShaders(shaderName);

        //start initializing the render data as soon as the shader is loaded
        glUseProgram(mShaderProgramId);

        Debug::Assert(materialData.IsArray(), "Data value of Json material file should be an array");

        //initialize all found uniforms
        for(rapidjson::SizeType i = 0; i < materialData.Size(); ++i)
        {
            const rapidjson::Value& entry = materialData[i];
            string name = entry["Name"].GetString();
            string type = entry["Type"].GetString();
            string value = entry["Value"].GetString();

            uniform_data unifData;

            if(type == "Texture2D")
            {
                GLuint* texId = new GLuint(SCETextures::LoadTexture(value));
                unifData.data = (void*)texId;
                unifData.type = UNIFORM_TEXTURE2D;
            }
            else if(type == "float")
            {
                unifData.type = UNIFORM_FLOAT;
                unifData.data = new float(Parser::StringToFloat(value));
            }
            else if (type == "vec4")
            {
                unifData.type = UNIFORM_VEC4;
                unifData.data = new vec4(Parser::StringToVec4(value));
            }
            else if (type == "vec3")
            {
                unifData.type = UNIFORM_VEC3;
                unifData.data = new vec3(Parser::StringToVec3(value));
            }
            unifData.name = name;
            unifData.dataID = glGetUniformLocation(mShaderProgramId, name.c_str());

            mUniforms[name] = unifData;
        }
    }
    else
    {
        Debug::RaiseError("Failed to open file " + fullPath);
    }
}

void Material::BindMaterialData()
{
    glUseProgram(mShaderProgramId);

    GLuint textureUnit = 0;

    map<string, uniform_data>::iterator it;
    for(it = mUniforms.begin(); it != mUniforms.end(); ++it)
    {
        uniform_data& uniform = it->second;
        switch(uniform.type) {
        case UNIFORM_FLOAT :
        {
            float f = *(float*)uniform.data;
            glUniform1f(uniform.dataID, f);
            break;
        }
        case UNIFORM_TEXTURE2D :
        {
            GLuint texId = *((GLuint*)uniform.data);
            SCETextures::BindTexture(texId, textureUnit, uniform.dataID);
            ++textureUnit;
            break;
        }
        case UNIFORM_VEC3 :
        {
            vec3 v = *(vec3*)uniform.data;
            glUniform3f(uniform.dataID, v.x, v.y, v.z);
            break;
        }
        case UNIFORM_VEC4 :
        {
            vec4 v = *(vec4*)uniform.data;
            glUniform4f(uniform.dataID, v.x, v.y, v.z, v.w);
            break;
        }
        }
    }
}

void Material::ReloadMaterial()
{
    SCE::Debug::RaiseError("Not implemented yet");
}

void Material::CleanMaterial()
{
    SCE::Debug::RaiseError("Not implemented yet");
}

const GLuint& Material::GetShaderProgram() const
{
    return mShaderProgramId;
}

GLuint Material::loadShaders(const string &filename)
{
    return SCEShaders::CreateShaderProgram(filename);
}


