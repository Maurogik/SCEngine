/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.cpp **********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Light.hpp"
#include "../headers/Container.hpp"
#include "../headers/Transform.hpp"
#include "../headers/Scene.hpp"


using namespace std;
using namespace glm;
using namespace SCE;


Light::Light(Handle<Container> &container, const std::string& typeName)
    : Component(container, "Light::" + typeName), mLightColor(1.0f, 1.0f, 1.0f, 1.0f)
{
    Scene::RegisterLight(Handle<Light>(this));
}

Light::~Light()
{
    Scene::UnregisterLight(Handle<Light>(this));
}

void Light::InitRenderDataForShader(const GLuint &shaderId)
{
    GLuint lightposUnif = glGetUniformLocation(shaderId, LIGHT_POS_UNIFORM_STR);
    GLuint lightcolorUnif = glGetUniformLocation(shaderId, LIGHT_COLOR_UNIFORM_STR);
    if(mLightColorByShader.count(shaderId) == 0){
        mLightPosByShader[shaderId] = lightposUnif;
        mLightColorByShader[shaderId] = lightcolorUnif;
    }
}

void Light::BindRenderDataForShader(const GLuint &shaderId)
{
    GLuint lightPosUnif = mLightPosByShader[shaderId];
    GLuint lightColorUnif = mLightColorByShader[shaderId];

    Handle<Transform> transform = GetContainer()->GetComponent<Transform>();

    vec3 lightPos = transform->GetWorldPosition();

    glUniform3f(lightPosUnif, lightPos.x, lightPos.y, lightPos.z);
    glUniform4f(lightColorUnif, mLightColor.r, mLightColor.g, mLightColor.b, mLightColor.a);
}



