/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEShaders.cpp**********/
/**************************************/

#include "../headers/SCEShaders.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCECore.hpp"

#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;


#define SCREEN_SIZE_UNIFORM_NAME "SCE_ScreenSize"
#define TIME_UNIFORM_NAME "SCE_TimeInSeconds"
#define DELTA_TIME_UNIFORM_NAME "SCE_DeltaTime"
#define DEBUG_SHADER_NAME "DebugShader"

namespace SCE
{

namespace ShaderUtils
{
    namespace
    {
        enum ShaderType
        {
            FRAGMENT_SHADER = 0,
            VERTEX_SHADER,
            TESSELATION_EVALUATION_SHADER,
            TESSELATION_CONTROL_SHADER,
            GEOMETRY_SHADER,
            COMPUTE_SHADER,
            SHADER_TYPE_COUNT
        };

        //Only one for now, but there will probably be more default uniforms added later
        struct DefaultUniforms
        {
            GLint screenSizeUniform;
            GLint MVPMatrixUniform;
            GLint ProjectionMatrixUniform;
            GLint ViewMatrixUniform;
            GLint ModelMatrixUniform;
            GLint timeUniform;
            GLint deltaTImeUniform;
        };

        //Only here to allow for automatic creation/destruction of data
        struct ShadersData
        {
            ShadersData()
                : compiledPrograms(),
                  defaultUniforms()
            {}

            ~ShadersData()
            {
                Internal::Log("Cleaning up shader system, will delete compiled shader promgrams");
                auto beginIt = begin(compiledPrograms);
                auto endIt = end(compiledPrograms);
                for(auto iterator = beginIt; iterator != endIt; iterator++) {
                    Internal::Log("Deleting shader : " + iterator->first);
                    glDeleteProgram(iterator->second);
                }
            }

            std::map<std::string, GLuint>       compiledPrograms;
            std::map<GLuint, DefaultUniforms>   defaultUniforms;
        };

        //Compilation unit scope variables
        ShadersData shaderData;
        bool shaderDebugEnabled = false;
        GLuint debugShaderProgram = GL_INVALID_INDEX;

        std::string shaderTypeToString(int shaderType)
        {
            using namespace SCE;
            string str;

            switch ((ShaderType)shaderType) {
            case ShaderType::FRAGMENT_SHADER :
                str = "Fragment Shader";
                break;
            case ShaderType::VERTEX_SHADER :
                str = "Vertex Shader";
                break;
            case ShaderType::GEOMETRY_SHADER:
                str = "Geometry Shader";
                break;
            case ShaderType::TESSELATION_CONTROL_SHADER:
                str = "TCS Shader";
                break;
            case ShaderType::TESSELATION_EVALUATION_SHADER:
                str = "TES Shader";
                break;
            case ShaderType::COMPUTE_SHADER:
                str = "Compute Shader";
                break;
            default:
                str = "Unknown Shader";
                break;
            }

            return str;
        }

        bool attachShaderToProgram(GLuint programID, const string& shaderFileName)
        {
            // Read the Shader code from the text file
            string shaderCodes[SHADER_TYPE_COUNT];
            GLuint shaderIds[SHADER_TYPE_COUNT];
            for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
            {   //init to max value
                shaderIds[i] = GL_INVALID_INDEX;
            }
            int currentShaderType = -1;

            string fullPath = RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;
            if(!ifstream(fullPath.c_str()))
            {
                fullPath = ENGINE_RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;
            }

            std::ifstream shaderStream(fullPath.c_str(), std::ios::in);

            int shadersTypeStartLine[SHADER_TYPE_COUNT] = {0};

            if(shaderStream.is_open())
            {
                std::string line = "";
                int lineCount = 0;

                while(getline(shaderStream, line))
                {
                    ++lineCount;

                    if(line.find("[VertexShader]") != string::npos)
                    {
                        currentShaderType = VERTEX_SHADER;
                        shaderIds[currentShaderType] = glCreateShader(GL_VERTEX_SHADER);
                        shadersTypeStartLine[currentShaderType] = lineCount;
                    }
                    else if(line.find("[FragmentShader]") != string::npos)
                    {
                        currentShaderType = FRAGMENT_SHADER;
                        shaderIds[currentShaderType] = glCreateShader(GL_FRAGMENT_SHADER);
                        shadersTypeStartLine[currentShaderType] = lineCount;
                    }
                    else if(line.find("[TES]") != string::npos)
                    {
                        currentShaderType = TESSELATION_EVALUATION_SHADER;
                        shaderIds[currentShaderType] = glCreateShader(GL_TESS_EVALUATION_SHADER);
                        shadersTypeStartLine[currentShaderType] = lineCount;
                    }
                    else if(line.find("[TCS]") != string::npos)
                    {
                        currentShaderType = TESSELATION_CONTROL_SHADER;
                        shaderIds[currentShaderType] = glCreateShader(GL_TESS_CONTROL_SHADER);
                        shadersTypeStartLine[currentShaderType] = lineCount;
                    }
                    else if(line.find("[Geometry]") != string::npos)
                    {
                        currentShaderType = GEOMETRY_SHADER;
                        shaderIds[currentShaderType] = glCreateShader(GL_GEOMETRY_SHADER);
                        shadersTypeStartLine[currentShaderType] = lineCount;
                    }
                    else if(line.find("[ComputeShader]") != string::npos)
                    {
                        currentShaderType = COMPUTE_SHADER;
                        shaderIds[currentShaderType] = glCreateShader(GL_COMPUTE_SHADER);
                        shadersTypeStartLine[currentShaderType] = lineCount;
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
                return false;
            }

            GLint result = GL_FALSE;
            int infoLogLength;

            Internal::Log("Compiling shader at " + fullPath);

            for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
            {
                if(shaderIds[i] != GL_INVALID_INDEX)
                {
                    // Compile Shader
                    const char* codePointer = shaderCodes[i].c_str();
                    glShaderSource(shaderIds[i], 1, &codePointer , NULL);
                    glCompileShader(shaderIds[i]);

                    // Check Shader
                    glGetShaderiv(shaderIds[i], GL_COMPILE_STATUS, &result);
                    glGetShaderiv(shaderIds[i], GL_INFO_LOG_LENGTH, &infoLogLength);

                    if (!result && infoLogLength > 0 ){
                        std::vector<char> shaderErrorMessage(infoLogLength+1);
                        glGetShaderInfoLog(shaderIds[i], infoLogLength, NULL, &shaderErrorMessage[0]);
                        Internal::Log("Compilation Error on " + shaderTypeToString(i) + " !!!");
                        Internal::Log(std::to_string(shadersTypeStartLine[i]) + "+ " +
                                    string(&shaderErrorMessage[0]) + "\n");
                    }
                }
            }


            // Link the program
            Internal::Log("Linking shader program\n");

            for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
            {
                if(shaderIds[i] != GL_INVALID_INDEX)
                {
                    glAttachShader(programID, shaderIds[i]);
                }
            }

            glLinkProgram(programID);

            // Check the linked program
            glGetProgramiv(programID, GL_LINK_STATUS, &result);
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (!result && infoLogLength > 0 ){
                std::vector<char> ProgramErrorMessage(infoLogLength+1);
                glGetProgramInfoLog(programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
                Internal::Log("Linking error !!!");
                Internal::Log(string(&ProgramErrorMessage[0]) + "\n");
            }

            //Now that the program is linked, we can delete the individual shaders
            for(int i = 0; i < SHADER_TYPE_COUNT; ++i)
            {
                if(shaderIds[i] != GL_INVALID_INDEX)
                {
                    glDeleteShader(shaderIds[i]);
                }
            }

            shaderData.compiledPrograms[shaderFileName] = programID;

            DefaultUniforms uniforms;
            uniforms.screenSizeUniform          = glGetUniformLocation(programID, SCREEN_SIZE_UNIFORM_NAME);
            uniforms.timeUniform                = glGetUniformLocation(programID, TIME_UNIFORM_NAME);
            uniforms.deltaTImeUniform           = glGetUniformLocation(programID, DELTA_TIME_UNIFORM_NAME);
            uniforms.MVPMatrixUniform           = glGetUniformLocation(programID, "MVP");
            uniforms.ViewMatrixUniform          = glGetUniformLocation(programID, "V");
            uniforms.ModelMatrixUniform         = glGetUniformLocation(programID, "M");
            uniforms.ProjectionMatrixUniform    = glGetUniformLocation(programID, "P");

            shaderData.defaultUniforms[programID] = uniforms;

            return true;
        }
    }

    //Loads and parses shader file
    GLuint CreateShaderProgram(const string& shaderFileName)
    {
        //shader has already been compiled
        if(shaderData.compiledPrograms.count(shaderFileName) > 0)
        {
            Internal::Log("Shader " + shaderFileName + " already compiled, using it directly");
            return shaderData.compiledPrograms[shaderFileName];
        }

        GLuint programID = glCreateProgram();

        bool success = attachShaderToProgram(programID, shaderFileName);
        //clean up and return if failed
        if(!success)
        {
            glDeleteProgram(programID);
            return GL_INVALID_INDEX;
        }

        return programID;
    }

    void DeleteShaderProgram(GLuint shaderId)
    {
        auto it = find_if(begin(shaderData.compiledPrograms),
                          end(shaderData.compiledPrograms),
                          [&shaderId](std::pair<string, GLuint> entry)
        { return entry.second == shaderId; } );

        if(it != end(shaderData.compiledPrograms))
        {
            Internal::Log("Delete program : " + it->first);
            glDeleteProgram(it->second);
            shaderData.compiledPrograms.erase(it);
        }

        auto itUniform = shaderData.defaultUniforms.find(shaderId);
        if(itUniform != end(shaderData.defaultUniforms))
        {
            shaderData.defaultUniforms.erase(itUniform);
        }
    }

    void BindDefaultUniforms(GLuint shaderId, const glm::mat4& modelMatrix,
                             const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
    {
        float width = (float)SCECore::GetWindowWidth();
        float height = (float)SCECore::GetWindowHeight();
        float timeInSecond = (float)SCE::Time::TimeInSeconds();
        float deltaTime = (float)SCE::Time::DeltaTime();

        DefaultUniforms& uniforms = shaderData.defaultUniforms[shaderId];

        glUniform2f(uniforms.screenSizeUniform, width, height);
        glUniform1f(uniforms.timeUniform, timeInSecond);
        glUniform1f(uniforms.deltaTImeUniform, deltaTime);

        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(uniforms.MVPMatrixUniform, 1, GL_FALSE, &(MVP[0][0]));
        glUniformMatrix4fv(uniforms.ModelMatrixUniform, 1, GL_FALSE, &(modelMatrix[0][0]));
        glUniformMatrix4fv(uniforms.ViewMatrixUniform, 1, GL_FALSE, &(viewMatrix[0][0]));
        glUniformMatrix4fv(uniforms.ProjectionMatrixUniform, 1, GL_FALSE, &(projectionMatrix[0][0]));
    }

    void UseShader(GLuint shaderProgram)
    {
//        if(shaderDebugEnabled)
//        {
//            glUseProgram(debugShaderProgram);
//        }
//        else
        {
            glUseProgram(shaderProgram);
        }
    }

#ifdef SCE_DEBUG_ENGINE

    bool ToggleDebugShader()
    {
        shaderDebugEnabled = !shaderDebugEnabled;
        if(shaderDebugEnabled && debugShaderProgram == GL_INVALID_INDEX)
        {
            debugShaderProgram = CreateShaderProgram(DEBUG_SHADER_NAME);
        }
        return shaderDebugEnabled;
    }

#define MAX_ATTACHED_SHADERS 6

    void ReloadShaders()
    {
        GLuint shaders[MAX_ATTACHED_SHADERS];
        GLsizei shaderCount = 0;

        for(auto shaderPair : shaderData.compiledPrograms)
        {
            GLuint programID = shaderPair.second;
            glGetAttachedShaders(programID, MAX_ATTACHED_SHADERS, &shaderCount, &shaders[0]);
            for(GLsizei i = 0; i < shaderCount; ++i)
            {
                glDetachShader(programID, shaders[i]);
            }
            attachShaderToProgram(programID, shaderPair.first);
        }
    }

#endif
}

}





