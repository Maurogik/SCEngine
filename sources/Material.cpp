/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.cpp ********/
/**************************************/

#include "../headers/Material.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/Scene.hpp"

#include "external/rapidjson/document.h" // rapidjson's DOM-style API
#include "external/rapidjson/prettywriter.h" // for stringify JSON
#include "external/rapidjson/filestream.h" // wrapper of C stream for prettywriter as output

#include <string>
#include <vector>
#include <iostream>
#include <fstream>


using namespace SCE;
using namespace std;



Material::Material(Handle<Container> &container, const string &filename, const string &typeName)
    : Component(container, "Material::" + typeName)
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
            string* fData = (string*) data.data;
            delete(fData);
        }
    }

    //unload shader
    glDeleteProgram(mProgramShaderId);
}

void Material::LoadMaterial(const string &filename)
{
    string fullPath = RESSOURCE_PATH + filename + MATERIAL_SUFIX;

    rapidjson::Document root;
    string fileStr = "";
    ifstream fileStream(fullPath.c_str(), ios::in);

    if(fileStream.is_open()){
        string currLine;
        while (getline(fileStream, currLine)) {
            fileStr += "\n" + currLine;
        }
        fileStream.close();

        if (root.Parse<0>(fileStr.c_str()).HasParseError()){
            Debug::RaiseError("Error reading the material file : " + filename);
        }

        //Access values
        Debug::Assert(root.IsObject(), "Malformated Json material file\n");

        string name = root["Name"].GetString();
        SCEInternal::InternalMessage("Parsing material : " + name);

        rapidjson::Value& materialRoot = root["Material"];
        rapidjson::Value& materialData = materialRoot["Data"];
        string shaderName   = root["Shader"].GetString();

        Debug::Assert(materialData.IsArray(), "Data value of Json material file should be an array");

        for(rapidjson::SizeType i = 0; i < materialData.Size(); ++i){
            const rapidjson::Value& entry = materialData[i];
            string name = entry["Name"].GetString();
            string type = entry["Type"].GetString();
            string value = entry["Value"].GetString();

            uniform_data unifData;

            if(type == "float"){
                unifData.type = UNIFORM_FLOAT;
                unifData.data = new float(Parser::StringToFloat(value));
            } else if (type == "Texture2D"){
                unifData.type = UNIFORM_TEXTURE2D;
                unifData.data = new string(value);
            } else if (type == "vec4"){
                unifData.type = UNIFORM_VEC4;
                unifData.data = new vec4(Parser::StringToVec4(value));
            } else if (type == "vec3"){
                unifData.type = UNIFORM_VEC3;
                unifData.data = new vec3(Parser::StringToVec3(value));
            }

            mUniforms[name] = unifData;
        }

        mProgramShaderId = loadShaders(shaderName);

    } else {
        Debug::RaiseError("Failed to open file " + fullPath);
    }
}

void Material::InitRenderData()
{
/*
    // Get a handle for our "MVP" uniform
    mMVPMatrixID    = glGetUniformLocation(mProgramID, "MVP");
    mViewMatrixID   = glGetUniformLocation(mProgramID, "V");
    mModelMatrixID  = glGetUniformLocation(mProgramID, "M");
 */
    map<string, uniform_data>::iterator it;
    for(it = mUniforms.begin(); it != mUniforms.end(); ++it){
        // iterator->first = key
        // iterator->second = value
        string name = it->first;
        uniform_data& uniform = it->second;

        uniform.dataID = glGetUniformLocation(mProgramShaderId, name.c_str());
        if(uniform.type == UNIFORM_TEXTURE2D){
            //do additional texture work (fetch sampler & stuff)
        }

    }

    Scene::InitLightRenderData(mProgramShaderId);
}

void Material::BindRenderData()
{
    Scene::BindLightRenderData(mProgramShaderId);
/*
    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(mMVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(mModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(mViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
 */
    map<string, uniform_data>::iterator it;
    for(it = mUniforms.begin(); it != mUniforms.end(); ++it){
        // iterator->first = key
        // iterator->second = value
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
            //exture uniforms not handled yet
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

}

void Material::CleanMaterial()
{

}

const GLuint& Material::GetShaderProgram() const
{
    return mProgramShaderId;
}

GLuint Material::loadShaders(const string &filename)
{
    string fullPath = SHADER_PATH + filename + SHADER_SUFIX;

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Shader code from the text file
    std::string shaderCode;
    string vertexCode, fragmentCode;

    bool vertex = true;

    std::ifstream shaderStream(fullPath.c_str(), std::ios::in);
    if(shaderStream.is_open()){
        std::string Line = "";

        while(getline(shaderStream, Line)){

            shaderCode += "\n" + Line;

            if(Line.find("VertexShader") != string::npos
                || Line.find("_{") != string::npos
                || Line.find("_}") != string::npos){
                continue;
            }

            if(Line.find("FragmentShader") != string::npos){
                vertex = false;
                continue;
            }

            if(vertex){
                vertexCode += "\n" + Line;
            } else {
                fragmentCode += "\n" + Line;
            }
        }
        shaderStream.close();
    }else{
        Debug::RaiseError("Failled to open file " + fullPath);
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    SCEInternal::InternalMessage("Vertex shader : \n\n" + vertexCode);
    SCEInternal::InternalMessage("Fragment shader : \n\n" + fragmentCode);


    SCEInternal::InternalMessage("Compiling shader at " + fullPath);
    // Compile Vertex Shader
    const char* vertexCodePointer = vertexCode.c_str();
    glShaderSource(VertexShaderID, 1, &vertexCodePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        SCEInternal::InternalMessage(string(&VertexShaderErrorMessage[0]) + "\n");
    }



    // Compile Fragment Shader
    const char* fragmentCodePointer = fragmentCode.c_str();
    glShaderSource(FragmentShaderID, 1, &fragmentCodePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        SCEInternal::InternalMessage(string(&FragmentShaderErrorMessage[0]) + "\n");
    }



    // Link the program
    SCEInternal::InternalMessage("Linking shader program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        SCEInternal::InternalMessage(string(&ProgramErrorMessage[0]) + "\n");
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}


