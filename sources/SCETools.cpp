/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETools.hpp ********/
/**************************************/

#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"

#include <assert.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace glm;


namespace SCE {

namespace Debug {

    void Assert(bool condition, const string &message)
    {
    #ifdef SCE_DEBUG
        if(!condition){
            RaiseError("Assertion failed : " + message);
        }
    #endif
    }

    void RaiseError(const string &errorMsg)
    {
    #ifdef SCE_DEBUG
        LogError(errorMsg);
        abort();
    #endif
    }

    void Log(const string &message)
    {
        cout << message << endl;
    }

    void LogError(const string &message)
    {
        cerr << "[ERROR] " << message << endl;
    }

}


namespace Parser {

    vec3 StringToVec3(const string &str)
    {
        float v1 = -666.0f, v2 = -666.0f, v3 = -666.0f;
        sscanf(str.c_str(), "(%f,%f,%f)", &v1, &v2, &v3);
        return vec3(v1, v2, v3);
    }


    vec4 StringToVec4(const string &str)
    {
        float v1 = -666.0f, v2 = -666.0f, v3 = -666.0f, v4 = -666.0f;
        sscanf(str.c_str(), "(%f,%f,%f,%f)", &v1, &v2, &v3, &v4);
        return vec4(v1, v2, v3, v4);
    }


    float StringToFloat(const string &str)
    {
        return (float)atof(str.c_str());
    }


    double StringToDouble(const string &str)
    {
        return (double)atof(str.c_str());
    }


    int StringToInt(const string &str)
    {
        return (int)atoi(str.c_str());
    }

}


namespace Tools {

    int HashFromString(const string &str)
    {
        std::hash<std::string> hash_func;
        return hash_func(str);
    }

    string ToLowerCase(const string &str)
    {
        string res = str;
        std::transform(begin(res), end(res), begin(res), ::tolower);
        return res;
    }

    int floatToColorRange(float val)
    {
        return (int)(val * 255.0f);
    }

}

namespace ShaderTools {

    GLuint CompileShader(const string &shaderFileName)
    {
        Internal::Log("TODO : cache shaders once compiled to avoid compiling again");
        string fullPath = RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;

        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Shader code from the text file
        std::string shaderCode;
        string vertexCode, fragmentCode;

        bool vertex = true;

        if(!ifstream(fullPath.c_str())){
            fullPath = ENGINE_RESSOURCE_PATH + shaderFileName + SHADER_SUFIX;
        }

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


        //Internal::Log("Vertex shader : \n\n" + vertexCode);
        //Internal::Log("Fragment shader : \n\n" + fragmentCode);


        Internal::Log("Compiling shader at " + fullPath);
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
            Internal::Log(string(&VertexShaderErrorMessage[0]) + "\n");
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
            Internal::Log(string(&FragmentShaderErrorMessage[0]) + "\n");
        }



        // Link the program
        Internal::Log("Linking shader program\n");
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
            Internal::Log(string(&ProgramErrorMessage[0]) + "\n");
        }

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        return ProgramID;
    }

}

}









