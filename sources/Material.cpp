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
#include "../headers/SCETools.hpp"
#include "../headers/SCEMetadataParser.hpp"

#include <iostream>
#include <fstream>


using namespace SCE;
using namespace std;


Material::Material(SCEHandle<Container> &container, const string &filename, const string &typeName)
    : Component(container, "Material::" + typeName) ,
      mMaterialName("Material not loaded yet"),
      mShaderProgramId(GL_INVALID_INDEX),
      mUniforms()
{
    LoadMaterial(filename);
}


Material::~Material()
{
    //free each allocated structures
    for(auto it = begin(mUniforms); it != end(mUniforms); ++it)
    {
        deleteUniformData(it->second);
    }

    //unload shader
    SCE::ShaderUtils::DeleteShaderProgram(mShaderProgramId);
}

void Material::LoadMaterial(const string &filename)
{
    string fullPath = RESSOURCE_PATH + filename + MATERIAL_SUFIX;

    if(!ifstream(fullPath.c_str()))
    {
        fullPath = ENGINE_RESSOURCE_PATH + filename + MATERIAL_SUFIX;
    }

    ifstream materialFileStream(fullPath.c_str(), ios::in);

    if(materialFileStream.is_open())
    {
        string currLine;
        mMaterialName = filename;
        Internal::Log("Parsing material : " + mMaterialName);
        map<string, string> lineData;


        //read first line to get shader name
        getline(materialFileStream, currLine);
        lineData = MetadataParser::GetLineData(currLine);
        Debug::Assert(lineData.count("Shader") > 0, "Could not find shader name in material file");
        mShaderProgramId = loadShaders(lineData["Shader"]);
//        //start initializing the render data as soon as the shader is loaded

        //read the whole file
        while (getline(materialFileStream, currLine))
        {
            lineData = MetadataParser::GetLineData(currLine);
            Debug::Assert(lineData.count("Name") == 1 && lineData.count("Type") == 1
                          && lineData.count("Value") == 1,
                          "Failed to read uniform line in material file");
            string name = lineData["Name"];
            string type = lineData["Type"];
            string value = lineData["Value"];

            uniform_data unifData;

            if(type == "Texture2D")
            {
                GLuint* texId = new GLuint(SCE::TextureUtils::LoadTexture(value));
                unifData.data = (void*)texId;
                unifData.type = UNIFORM_TEXTURE2D;
            }
            else if(type == "float")
            {
                unifData.type = UNIFORM_FLOAT;
                unifData.data = new float(MetadataParser::StringToFloat(value));
            }
            else if (type == "vec4")
            {
                unifData.type = UNIFORM_VEC4;
                unifData.data = new vec4(MetadataParser::StringToVec4(value));
            }
            else if (type == "vec3")
            {
                unifData.type = UNIFORM_VEC3;
                unifData.data = new vec3(MetadataParser::StringToVec3(value));
            }
            unifData.name = name;
            unifData.dataID = glGetUniformLocation(mShaderProgramId, name.c_str());

            mUniforms[name] = unifData;
        }
        materialFileStream.close();
    }
    else
    {
        Debug::RaiseError("Failed to open file " + fullPath);
    }
}

void Material::BindMaterialData()
{
    SCE::ShaderUtils::UseShader(mShaderProgramId);

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
            SCE::TextureUtils::BindTexture(texId, textureUnit, uniform.dataID);
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
    LoadMaterial(mMaterialName);
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
    return SCE::ShaderUtils::CreateShaderProgram(filename);
}

void Material::deleteUniformData(uniform_data & data)
{
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
        SCE::TextureUtils::DeleteTexture(*fData);
        delete(fData);
    }
    data.data = nullptr;
}


