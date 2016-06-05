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
#include "../headers/SCELighting.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCEMeshLoader.hpp"

using namespace std;
using namespace glm;
using namespace SCE;

//#define LIGHT_BOUNDS_COMPLEX

#define REACH_DEFAULT 1.0f
#define COLOR_DEFAULT (glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
#define ANGLE_DEFAULT 45.0f

#define POINT_LIGHT_CUTOFF (1.0f/256.0f)
#define SPOT_LIGHT_CUTOFF (0.1f/256.0f)

#define LIGHT_ROUTINE_COUNT 1
#define LIGHT_ROUTINE_SHADER_TYPE GL_FRAGMENT_SHADER

#define COMPUTE_LIGHT_UNIFORM_NAME      "SCE_ComputeLight"
#define COMPUTE_DIRECTIONAL_LIGHT_NAME  "SCE_ComputeDirectionalLight"
#define COMPUTE_POINT_LIGHT_NAME        "SCE_ComputePointLight"
#define COMPUTE_SPOT_LIGHT_NAME         "SCE_ComputeSpotLight"

string lightUniformNames[LIGHT_UNIFORMS_COUNT] = {
    "SCE_LightPosition_worldspace",
    "SCE_LightDirection_worldspace",
    "SCE_LightReach_worldspace",
    "SCE_LightColor",
    "SCE_LightMaxDotAngle",
    "SCE_EyePosition_worldspace",
    "SCE_ShadowStrength"
};

Light::Light(SCEHandle<Container>& container, LightType lightType,
             const std::string& typeName)
    : Component(container, "Light::" + typeName),
      mLightType(lightType),
      mLightReach(REACH_DEFAULT),
      mLightMaxAngle(ANGLE_DEFAULT),
      mLightColor(COLOR_DEFAULT),
      mIsSunLight(false),
      mLightUniforms(),
      mLightMeshId(-1),
      mLightRenderer(nullptr)
{
    SCELighting::RegisterLight(SCEHandle<Light>(this));
    generateLightMesh();
    //change layer to avoid rendering the light mesh as a regular mesh
    container->SetLayer(LIGHTS_LAYER);
}

Light::~Light()
{
    SCELighting::UnregisterLight(SCEHandle<Light>(this));
}

LightType Light::GetLightType() const
{
    return mLightType;
}

//Will be called by the Lighting system upon light registration
void Light::InitLightRenderData(GLuint lightShaderProgram)
{
    //get the location for all uniforms, some may not be used but that's ok.
    for(int type = LIGHT_POSITION; type < LIGHT_UNIFORMS_COUNT; ++type)
    {
        string name = lightUniformNames[type];
        GLuint uniform = glGetUniformLocation(lightShaderProgram, name.c_str());
        //set the uniformId for this uniform type
        mLightUniforms[type] = uniform;
    }

    //Check the number of active subroutine uniforms
    //This is expecting that subroutines are only used for light in this shader
    GLint subroutineUniformCount;
    glGetProgramStageiv(lightShaderProgram, LIGHT_ROUTINE_SHADER_TYPE,
                        GL_ACTIVE_SUBROUTINE_UNIFORMS, &subroutineUniformCount);

    Debug::Assert(subroutineUniformCount == LIGHT_ROUTINE_COUNT,
                  string("Wrong number of subroutine found in light pass shader \n")
                  + "expected " + std::to_string(LIGHT_ROUTINE_COUNT)
                  + " found " + std::to_string(subroutineUniformCount));

    //Get what subroutine index corresponds  to this light type
    string lightSubroutineName("UnknownLightType");
    switch(mLightType)
    {
    case POINT_LIGHT :
        lightSubroutineName = COMPUTE_POINT_LIGHT_NAME;
        break;
    case SPOT_LIGHT :
        lightSubroutineName = COMPUTE_SPOT_LIGHT_NAME;
        break;
    case DIRECTIONAL_LIGHT :
        lightSubroutineName = COMPUTE_DIRECTIONAL_LIGHT_NAME;
        break;
    default :
        Debug::RaiseError(std::string("Unknown Light type : ") + std::to_string(mLightType));
        break;
    }

    mLightSubroutineIndex = glGetSubroutineIndex( lightShaderProgram,
                                                  LIGHT_ROUTINE_SHADER_TYPE,
                                                  lightSubroutineName.c_str());

    mLightRenderer = GetContainer()->AddComponent<MeshRenderer>(mLightMeshId);
}

void Light::bindRenderDataForShader(const vec3& cameraPosition)
{
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    vec3 lightPos = transform->GetScenePosition();
    vec3 lightDir = transform->Forward();
    lightDir = glm::normalize(lightDir);

    /* half angle because we want the dot from light Z to cone surfcade*/
    vec3 rotatedZ = glm::angleAxis(radians(mLightMaxAngle * 0.5f) , vec3(1.0, 0.0, 0.0)) * vec3(0.0, 0.0, 1.0);
    rotatedZ = normalize(rotatedZ);
    float maxDot = dot(rotatedZ, vec3(0.0, 0.0, 1.0));

    //send the light data to the uniforms
    for(int type = LIGHT_POSITION; type < LIGHT_UNIFORMS_COUNT; ++ type)
    {
        GLuint unifId = mLightUniforms[type];

        switch(type)
        {
        case LIGHT_POSITION :
            glUniform3f(unifId, lightPos.x, lightPos.y, lightPos.z);
            break;
        case LIGHT_DIRECTION :
            glUniform3f(unifId, lightDir.x, lightDir.y, lightDir.z);
            break;
        case LIGHT_REACH :
            glUniform1f(unifId, mLightReach);
            break;
        case LIGHT_COLOR :
            glUniform4f(unifId, mLightColor.r, mLightColor.g, mLightColor.b, mLightColor.a);
            break;
        case EYE_POSITION :
            glUniform3f(unifId, cameraPosition.x, cameraPosition.y, cameraPosition.z);
            break;
        case SHADOW_STRENGTH :
            glUniform1f(unifId, mIsSunLight ? 1.0f : 0.0f);
            break;
        case LIGHT_MAX_DOT :
            glUniform1f(unifId, maxDot);
            break;
        default :
            SCE::Debug::LogError(std::string("Unknown ligth uniform type : ") + std::to_string(type));
        }
    }
}

void Light::bindLightModelForShader()
{
    //send the binding of uniform/subroutines to the current context/shader
    //last param is array of subroutine index for each subroutine uniforms
    //array[i] = index of subroutine to pick for uniform i
    glUniformSubroutinesuiv(LIGHT_ROUTINE_SHADER_TYPE, LIGHT_ROUTINE_COUNT, &mLightSubroutineIndex);
}

const glm::vec4& Light::GetLightColor() const
{
    return mLightColor;
}

void Light::SetLightColor(const glm::vec4& lightColor)
{
    mLightColor = lightColor;
}

void Light::RenderWithLightData(const CameraRenderData& renderData)
{
    //compute light position to send it to the shader
    glm::mat4 camToWorld = glm::inverse(renderData.viewMatrix);
    vec4 camPos = camToWorld * vec4(0, 0, 0, 1);
    bindRenderDataForShader(vec3(camPos.x, camPos.y, camPos.z));
    bindLightModelForShader();

    mLightRenderer->Render(renderData, mLightType == DIRECTIONAL_LIGHT);
}

void Light::RenderWithoutLightData(const CameraRenderData& renderData)
{
    mLightRenderer->Render(renderData, mLightType == DIRECTIONAL_LIGHT);
}

void Light::SetIsSunLight(bool isSunLight)
{
    if(isSunLight)
    {
        Debug::Assert(mLightType == DIRECTIONAL_LIGHT, "Shadow casting only supported for directionnal lights");
        SCELighting::SetSunLight(SCEHandle<Light>(this));
    }
    else
    {
        SCELighting::SetSunLight(SCEHandle<Light>(nullptr));
    }
    mIsSunLight = isSunLight;
}

float Light::GetLightMaxAngle() const
{
    return mLightMaxAngle;
}

void Light::SetLightMaxAngle(float lightMaxAngle)
{
    mLightMaxAngle = lightMaxAngle;
    generateLightMesh();
}

float Light::GetLightReach() const
{
    return mLightReach;
}

void Light::SetLightReach(float lightReach)
{
    mLightReach = lightReach;
    generateLightMesh();
}

void Light::generateLightMesh()
{
    ui16 meshId = -1;
    switch(mLightType)
    {
    case POINT_LIGHT:
        meshId = generatePointLightMesh();
        break;
    case DIRECTIONAL_LIGHT :
        meshId = generateDirectionalLightMesh();
        break;
    case SPOT_LIGHT :
        meshId = generateSpotLightMesh();
        break;
    default :
        Debug::RaiseError("Unknow light type");
        break;
    }

    mLightMeshId = meshId;

    if(mLightRenderer)
    {
        mLightRenderer->UpdateRenderedMesh(meshId);
    }
}

ui16 Light::generateDirectionalLightMesh()
{
    return SCE::MeshLoader::CreateQuadMesh("Quad");
}

ui16 Light::generateSpotLightMesh()
{
    //compute spot attenuation
#ifdef LIGHT_BOUNDS_COMPLEX
    vec3 spotDir(0.0, 0.0, 1.0);
    vec3 spotAngleDir = normalize(angleAxis(radians(mLightMaxAngle), vec3(1.0, 0.0, 0.0)) * spotDir);
    float maxDot = dot(spotDir, spotAngleDir);
    mSpotAttenuation = log(mLightCutoff) / log(maxDot) * 2.0;
#endif

    SCEHandle<Container> container = GetContainer();
    ui16 meshId = SCE::MeshLoader::CreateConeMesh(mLightMaxAngle, 4.0f);

    SCEHandle<Transform> transform = container->GetComponent<Transform>();
    transform->SetLocalScale(vec3(mLightReach));

    return meshId;
}

ui16 Light::generatePointLightMesh()
{

#ifdef LIGHT_BOUNDS_COMPLEX
    //Compute radius at which light intensity goes under some threshold
    //using solution to quadratic equation 1/square(x/r + 1) = cutoff(c)
    //s = (-2*r + sqrt(4*r*r/c))/2
    float lightSphereRadius = 0.0f;
    float delta = (4.0f * mLightReach * mLightReach)/mLightCutoff;
    lightSphereRadius = (-2.0f * mLightReach  + sqrt(delta))/2.0f;
#else
    float lightSphereRadius = mLightReach;
#endif

    SCEHandle<Container> container = GetContainer();
    ui16 meshId = SCE::MeshLoader::CreateSphereMesh(4.0f);

    SCEHandle<Transform> transform = container->GetComponent<Transform>();
    transform->SetLocalScale(vec3(lightSphereRadius));

    return meshId;
}
