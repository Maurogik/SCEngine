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

namespace SCE
{

namespace ShaderUtils
{
    enum ShaderType
    {
        FRAGMENT_SHADER = 0,
        VERTEX_SHADER,
        TESSELATION_EVALUATION_SHADER,
        TESSELATION_CONTROL_SHADER,
        GEOMETRY_SHADER,
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

    //Compilation unit scope variable
    ShadersData shaderData;

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
        default:
            str = "Unknown Shader";
            break;
        }

        return str;
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
                if(line.find("[VertexShader]") != string::npos)
                {
                    currentShaderType = VERTEX_SHADER;
                    shaderIds[currentShaderType] = glCreateShader(GL_VERTEX_SHADER);
                }
                else if(line.find("[FragmentShader]") != string::npos)
                {
                    currentShaderType = FRAGMENT_SHADER;
                    shaderIds[currentShaderType] = glCreateShader(GL_FRAGMENT_SHADER);
                }
                else if(line.find("[TES]") != string::npos)
                {
                    currentShaderType = TESSELATION_EVALUATION_SHADER;
                    shaderIds[currentShaderType] = glCreateShader(GL_TESS_EVALUATION_SHADER);
                }
                else if(line.find("[TCS]") != string::npos)
                {
                    currentShaderType = TESSELATION_CONTROL_SHADER;
                    shaderIds[currentShaderType] = glCreateShader(GL_TESS_CONTROL_SHADER);
                }
                else if(line.find("[Geometry]") != string::npos)
                {
                    currentShaderType = GEOMETRY_SHADER;
                    shaderIds[currentShaderType] = glCreateShader(GL_GEOMETRY_SHADER);
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
                if (!result && infoLogLength > 0 ){
                    std::vector<char> shaderErrorMessage(infoLogLength+1);
                    glGetShaderInfoLog(shaderIds[i], infoLogLength, NULL, &shaderErrorMessage[0]);
                    Internal::Log("Compilation Error on " + shaderTypeToString(i) + " !!!");
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
            if(shaderIds[i] != GLuint(-1))
            {
                glDeleteShader(shaderIds[i]);
            }
        }

        shaderData.compiledPrograms[shaderFileName] = programID;

        DefaultUniforms uniforms;
        uniforms.screenSizeUniform          = glGetUniformLocation(programID, SCREEN_SIZE_UNIFORM_NAME);
        uniforms.MVPMatrixUniform           = glGetUniformLocation(programID, "MVP");
        uniforms.ViewMatrixUniform          = glGetUniformLocation(programID, "V");
        uniforms.ModelMatrixUniform         = glGetUniformLocation(programID, "M");
        uniforms.ProjectionMatrixUniform    = glGetUniformLocation(programID, "P");

        shaderData.defaultUniforms[programID] = uniforms;

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
        float width = SCECore::GetWindowWidth();
        float height = SCECore::GetWindowHeight();
        DefaultUniforms& uniforms = shaderData.defaultUniforms[shaderId];

        glUniform2f(uniforms.screenSizeUniform, width, height);

        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(uniforms.MVPMatrixUniform, 1, GL_FALSE, &(MVP[0][0]));
        glUniformMatrix4fv(uniforms.ModelMatrixUniform, 1, GL_FALSE, &(modelMatrix[0][0]));
        glUniformMatrix4fv(uniforms.ViewMatrixUniform, 1, GL_FALSE, &(viewMatrix[0][0]));
        glUniformMatrix4fv(uniforms.ProjectionMatrixUniform, 1, GL_FALSE, &(projectionMatrix[0][0]));
    }
}

}





