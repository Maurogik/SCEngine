/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.cpp **********/
/**************************************/

#include "../headers/Light.hpp"
#include "../headers/Container.hpp"
#include "../headers/Transform.hpp"

using namespace std;
using namespace glm;
using namespace SCE;


Light::Light() :
    mLightColor(1.0f, 1.0f, 1.0f, 1.0f)
{

}

Light::~Light()
{
}

void Light::SetContainer(Container *cont)
{
    //register light
}

void Light::InitRenderDataForShader(const GLuint &shaderId);
{
    //glGetUniformLocation(mProgramShaderId, name.c_str());
}

void Light::BindRenderDataForShader(const GLuint &shaderId);
{
    /*
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
    */
}



