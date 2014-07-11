/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.cpp ********/
/**************************************/

#include "../headers/Material.hpp"
#include "../headers/SCETools.hpp"

#include "external/rapidjson/document.h" // rapidjson's DOM-style API
#include "external/rapidjson/prettywriter.h" // for stringify JSON
#include "external/rapidjson/filestream.h" // wrapper of C stream for prettywriter as output

#include <string>
#include <vector>
#include <iostream>
#include <fstream>


using namespace SCE;
using namespace std;



Material::Material()
{

}

Material::~Material()
{
    //free each allocated structures
}

Material* Material::LoadMaterial(const string &filename)
{
    Material* material;

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
            return 0l;
        }

        //Access values
        SCE_ASSERT(root.IsObject(), "Malformated Json material file\n");

        string name = root["Name"].GetString();
        SCE_DEBUG_LOG("Parsing material : %s", name.c_str());

        rapidjson::Value& materialRoot = root["Material"];
        rapidjson::Value& materialData = materialRoot["Data"];
        string shaderName   = root["Shader"].GetString();

        material = new Material();

        SCE_ASSERT(materialData.IsArray(), "Data value of Json material file should be an array");

        for(rapidjson::SizeType i = 0; i < materialData.Size(); ++i){
            const rapidjson::Value& entry = materialData[i];
            string name = entry["Name"].GetString();
            string type = entry["Type"].GetString();
            string value = entry["Value"].GetString();

            uniform_data unifData;

            if(type == "float"){
                unifData.type = UNIFORM_FLOAT;
                unifData.data = new float(StringToFloat(value));
            } else if (type == "Texture2D"){
                unifData.type = UNIFORM_TEXTURE2D;
                unifData.data = new string(value);
            } else if (type == "vec4"){
                unifData.type = UNIFORM_VEC4;
                unifData.data = new vec4(StringToVec4(value));
            } else if (type == "vec3"){
                unifData.type = UNIFORM_VEC3;
                unifData.data = new vec3(StringToVec3(value));
            }

            material->mUniforms[name] = unifData;
        }

        material->mProgramShaderId = loadShaders(shaderName);

    } else {
        SCE_ERROR("Failed to open file %s", fullPath.c_str());
    }

    return material;
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

        }

    }
}

void Material::BindRenderData()
{
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
            //SCE_DEBUG_LOG("Texture uniforms not handled yet");
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

void Material::SetUniformValue(const string &uniformName, void *value)
{
    mUniforms[uniformName].data = value;
}

void *Material::GetUniformValue(const string &uniformName)
{
    return 0l;
}

GLuint Material::GetShaderProgram()
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
        SCE_ERROR("Failled to open file %s", fullPath.c_str());
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    //SCE_DEBUG_LOG("Shaders : \n\n%s", shaderCode.c_str());
    SCE_DEBUG_LOG("Vertex shader : \n\n%s", vertexCode.c_str());
    SCE_DEBUG_LOG("Fragment shader : \n\n%s", fragmentCode.c_str());


    SCE_DEBUG_LOG("Compiling shader at %s", fullPath.c_str());
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
        SCE_DEBUG_LOG("%s\n", &VertexShaderErrorMessage[0]);
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
        SCE_DEBUG_LOG("%s\n", &FragmentShaderErrorMessage[0]);
    }



    // Link the program
    SCE_DEBUG_LOG("Linking shader program\n");
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
        SCE_DEBUG_LOG("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
