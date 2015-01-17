/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.cpp **********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Light.hpp"
#include "../headers/Container.hpp"
#include "../headers/Transform.hpp"
#include "../headers/SCEScene.hpp"


using namespace std;
using namespace glm;
using namespace SCE;

#define REACH_DEFAULT 10.0f
#define COLOR_DEFAULT vec4(1.0f, 1.0f, 1.0f, 1.0f)
#define START_RADIUS_DEFAULT 1.0f
#define END_RADIUS_DEFAULT 5.0f

static string s_lightUniformNames[LIGHT_UNIFORMS_COUNT] = {
    "SCE_LightPosition_worldspace",
    "SCE_LightDirection_worldspace",
    "SCE_LightReach_worldspace",
    "SCE_LightColor",
    "SCE_LightStartRadius_worldspace",
    "SCE_LightEndRadius_worldspace"
};

Light::Light(SCEHandle<Container>& container, const LightType &lightType,
             const std::string& typeName = "")
    : Component(container, "Light::" + typeName),
      mLightType(lightType),
      mLightReach(REACH_DEFAULT),
      mLightStartRadius(START_RADIUS_DEFAULT),
      mLightEndRadius(END_RADIUS_DEFAULT),
      mLightColor(COLOR_DEFAULT)
{
    SCEScene::RegisterLight(SCEHandle<Light>(this));
}


Light::~Light()
{
    SCEScene::UnregisterLight(SCEHandle<Light>(this));
}

void Light::InitRenderDataForShader(const GLuint &shaderId)
{
    //get the location for all uniforms, some may not be used but that's ok.
    for(LightUniformType type = LIGHT_POSITION; type < LIGHT_UNIFORMS_COUNT; ++ type){
        GLuint uniform = glGetUniformLocation(shaderId, s_lightUniformNames[type]);
        //set the uniformId for this uniform and shader if it's not done already
        if(mLightUniformsByShader[type].count(shaderId == 0)){
            mLightUniformsByShader[type][shaderId] = uniform;
        }
    }
}

void Light::BindRenderDataForShader(const GLuint &shaderId)
{
    GLuint lightPosUnif = mLightPosByShader[shaderId];
    GLuint lightColorUnif = mLightColorByShader[shaderId];

    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    vec3 lightPos = transform->GetWorldPosition();
    vec3 lightDir = transform->GetWorldOrientation();

//    glUniform3f(lightPosUnif, lightPos.x, lightPos.y, lightPos.z);
//    glUniform4f(lightColorUnif, mLightColor.r, mLightColor.g, mLightColor.b, mLightColor.a);

    for(LightUniformType type = LIGHT_POSITION; type < LIGHT_UNIFORMS_COUNT; ++ type){
        GLuint unifId = mLightUniformsByShader[type][shaderId];

        switch(type){
        case LIGHT_POSITION :
            glUniform3f(unifId, lightPos.x, lightPos.y, lightPos.z);
            break;
        case LIGHT_DIRECTION :
            glUniform3f(unifId, lightDir.x, lightDir.y, lightDir.z);
            break;
        case LIGHT_REACH :
            break;
        case LIGHT_COLOR :
            break;
        case LIGHT_START_RADIUS :
            break;
        case LIGHT_END_RADIUS :
            break;
        }
    }

}

float Light::GetLightColor() const
{
    return mLightColor;
}

void Light::SetLightColor(float lightColor)
{
    mLightColor = lightColor;
}

float Light::GetLightEndRadius() const
{
    return mLightEndRadius;
}

void Light::SetLightEndRadius(float lightEndRadius)
{
    mLightEndRadius = lightEndRadius;
}

float Light::GetLightStartRadius() const
{
    return mLightStartRadius;
}

void Light::SetLightStartRadius(float lightStartRadius)
{
    mLightStartRadius = lightStartRadius;
}

float Light::GetLightReach() const
{
    return mLightReach;
}

void Light::SetLightReach(float lightReach)
{
    mLightReach = lightReach;
}


