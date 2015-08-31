/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEShaders.cpp**********/
/**************************************/

#include "../headers/SCEShaders.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCECore.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>

using namespace SCE;
using namespace std;


#define SCREEN_SIZE_UNIFORM_NAME "SCE_ScreenSize"



SCEShaders* SCEShaders::s_instance = nullptr;

SCEShaders::SCEShaders()
    : mCompiledShaderPrograms(),
      mDefaultUniforms()
{

}

SCEShaders::~SCEShaders()
{
    Internal::Log("Cleaning up shader system, will delete compiled shader promgrams");
    auto beginIt = begin(s_instance->mCompiledShaderPrograms);
    auto endIt = end(s_instance->mCompiledShaderPrograms);
    for(auto iterator = beginIt; iterator != endIt; iterator++) {
        Internal::Log("Deleting shader : " + iterator->first);
        glDeleteProgram(iterator->second);
    }
}

void SCEShaders::Init()
{
    Debug::Assert(!s_instance, "An instance of the Shader system already exists");
    s_instance = new SCEShaders();
}

void SCEShaders::CleanUp()
{
    Debug::Assert(s_instance, "No Shader system instance found, Init the system before using it");
    delete s_instance;
}

GLuint SCEShaders::CreateShaderProgram(const string& shaderFileName)
{
    Debug::Assert(s_instance, "No Shader system instance found, Init the system before using it");

    //shader has already been compiled
    if(s_instance->mCompiledShaderPrograms.count(shaderFileName) > 0)
    {
        Internal::Log("Shader " + shaderFileName + " already compiled, using it directly");
        return s_instance->mCompiledShaderPrograms[shaderFileName];
    }

    Internal::Log("TODO : cache shaders once compiled to avoid compiling again");
    string fullPath = RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;

    // Create the shaders
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Shader code from the text file
    std::string shaderCode;
    string vertexCode, fragmentCode;

    bool vertex = true;

    if(!ifstream(fullPath.c_str()))
    {
        fullPath = ENGINE_RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;
    }

    std::ifstream shaderStream(fullPath.c_str(), std::ios::in);

    if(shaderStream.is_open())
    {
        std::string Line = "";

        while(getline(shaderStream, Line))
        {

            shaderCode += "\n" + Line;

            if(Line.find("VertexShader") != string::npos
                || Line.find("_{") != string::npos
                || Line.find("_}") != string::npos)
            {
                continue;
            }

            if(Line.find("FragmentShader") != string::npos)
            {
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
    }
    else
    {
        Debug::RaiseError("Failled to open file " + fullPath);
        return 0;
    }

    GLint result = GL_FALSE;
    int infoLogLength;

    Internal::Log("Compiling shader at " + fullPath);
    // Compile Vertex Shader
    const char* vertexCodePointer = vertexCode.c_str();
    glShaderSource(vertexShaderID, 1, &vertexCodePointer , NULL);
    glCompileShader(vertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if ( infoLogLength > 0 ){
        std::vector<char> vertexShaderErrorMessage(infoLogLength+1);
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
        Internal::Log(string(&vertexShaderErrorMessage[0]) + "\n");
    }

    // Compile Fragment Shader
    const char* fragmentCodePointer = fragmentCode.c_str();
    glShaderSource(fragmentShaderID, 1, &fragmentCodePointer , NULL);
    glCompileShader(fragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if ( infoLogLength > 0 ){
        std::vector<char> fragmentShaderErrorMessage(infoLogLength+1);
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
        Internal::Log(string(&fragmentShaderErrorMessage[0]) + "\n");
    }

    // Link the program
    Internal::Log("Linking shader program\n");
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if ( infoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(infoLogLength+1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
        Internal::Log(string(&ProgramErrorMessage[0]) + "\n");
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    s_instance->mCompiledShaderPrograms[shaderFileName] = programID;

    DefaultUniforms uniforms;
    uniforms.screenSizeUniform = glGetUniformLocation(programID, SCREEN_SIZE_UNIFORM_NAME);
    s_instance->mDefaultUniforms[programID] = uniforms;

    return programID;
}

void SCEShaders::DeleteShaderProgram(GLuint shaderId)
{
    Debug::Assert(s_instance, "No Shader system instance found, Init the system before using it");

    auto it = find_if(begin(s_instance->mCompiledShaderPrograms),
                      end(s_instance->mCompiledShaderPrograms),
                      [&shaderId](std::pair<string, GLuint> entry)
    { return entry.second == shaderId; } );

    if(it != end(s_instance->mCompiledShaderPrograms))
    {
        Internal::Log("Delete program : " + it->first);
        glDeleteProgram(it->second);
        s_instance->mCompiledShaderPrograms.erase(it);
    }

    auto itUniform = s_instance->mDefaultUniforms.find(shaderId);
    if(itUniform != end(s_instance->mDefaultUniforms))
    {
        s_instance->mDefaultUniforms.erase(itUniform);
    }
}

void SCEShaders::BindDefaultUniforms(GLuint shaderId)
{
    Debug::Assert(s_instance, "No Shader system instance found, Init the system before using it");

    float width = SCECore::GetWindowWidth();
    float height = SCECore::GetWindowHeight();

    glUniform2f(s_instance->mDefaultUniforms[shaderId].screenSizeUniform, width, height);
}


