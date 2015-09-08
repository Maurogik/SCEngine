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

    // Read the Shader code from the text file
    string shaderCodes[SHADER_TYPE_COUNT];
    GLuint shaderIds[SHADER_TYPE_COUNT];
    for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
    {   //init to max value
        shaderIds[i] = GLuint(-1);
    }

    int currentShaderType = -1;

    if(!ifstream(fullPath.c_str()))
    {
        fullPath = ENGINE_RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;
    }

    std::ifstream shaderStream(fullPath.c_str(), std::ios::in);

    if(shaderStream.is_open())
    {
        std::string line = "";

        while(getline(shaderStream, line))
        {
            if(line.find("VertexShader") != string::npos)
            {
                currentShaderType = VERTEX_SHADER;
                shaderIds[currentShaderType] = glCreateShader(GL_VERTEX_SHADER);
            }
            else if(line.find("FragmentShader") != string::npos)
            {
                currentShaderType = FRAGMENT_SHADER;
                shaderIds[currentShaderType] = glCreateShader(GL_FRAGMENT_SHADER);
            }
            else if(line.find("TES") != string::npos)
            {
                currentShaderType = TESSELATION_EVALUATION_SHADER;
                shaderIds[currentShaderType] = glCreateShader(GL_TESS_EVALUATION_SHADER);
            }
            else if(line.find("TCS") != string::npos)
            {
                currentShaderType = TESSELATION_CONTROL_SHADER;
                shaderIds[currentShaderType] = glCreateShader(GL_TESS_CONTROL_SHADER);
            }
            else if(line.find("_{") == string::npos &&
                    line.find("_}") == string::npos &&
                    currentShaderType >= 0)
            {                
                shaderCodes[currentShaderType] += "\n" + line;
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

    for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
    {
        if(shaderIds[i] != GLuint(-1))
        {
            // Compile Shader
            const char* codePointer = shaderCodes[i].c_str();
            glShaderSource(shaderIds[i], 1, &codePointer , NULL);
            glCompileShader(shaderIds[i]);

            // Check Shader
            glGetShaderiv(shaderIds[i], GL_COMPILE_STATUS, &result);
            glGetShaderiv(shaderIds[i], GL_INFO_LOG_LENGTH, &infoLogLength);
            if ( infoLogLength > 0 ){
                std::vector<char> shaderErrorMessage(infoLogLength+1);
                glGetShaderInfoLog(shaderIds[i], infoLogLength, NULL, &shaderErrorMessage[0]);
                Internal::Log(string(&shaderErrorMessage[0]) + "\n");
            }
        }
    }


    // Link the program
    Internal::Log("Linking shader program\n");
    GLuint programID = glCreateProgram();

    for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
    {
        if(shaderIds[i] != GLuint(-1))
        {
            glAttachShader(programID, shaderIds[i]);
        }
    }

    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if ( infoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(infoLogLength+1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
        Internal::Log(string(&ProgramErrorMessage[0]) + "\n");
    }

    //Now that the program is linked, we can delete the individual shaders
    for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
    {
        if(shaderIds[i] != GLuint(-1))
        {
            glDeleteShader(shaderIds[i]);
        }
    }

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


