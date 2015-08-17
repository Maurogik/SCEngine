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
#include "../headers/SCECore.hpp"

using namespace std;
using namespace glm;
using namespace SCE;

#define REACH_DEFAULT 1.0f
#define COLOR_DEFAULT vec4(1.0f, 1.0f, 1.0f, 1.0f)
#define ANGLE_DEFAULT 45.0f

#define POINT_LIGHT_CUTOFF (1.0/256.0)

#define LIGHT_ROUTINE_COUNT 1

#define SCREEN_SIZE_UNIFORM_NAME        "SCE_ScreenSize"
#define COMPUTE_LIGHT_UNIFORM_NAME      "SCE_ComputeLight"
#define COMPUTE_DIRECTIONAL_LIGHT_NAME  "SCE_ComputeDirectionalLight"
#define COMPUTE_POINT_LIGHT_NAME        "SCE_ComputePointLight"
#define COMPUTE_SPOT_LIGHT_NAME         "SCE_ComputeSpotLight"

string s_lightUniformNames[LIGHT_UNIFORMS_COUNT] = {
    "SCE_LightPosition_worldspace",
    "SCE_LightDirection_worldspace",
    "SCE_LightReach_worldspace",
    "SCE_LightColor",
    "SCE_LightMaxAngle",
    "SCE_LightCutoff"
};

Light::Light(SCEHandle<Container>& container, const LightType &lightType,
             const std::string& typeName)
    : Component(container, "Light::" + typeName),
      mLightType(lightType),
      mLightReach(REACH_DEFAULT),
      mLightMaxAngle(ANGLE_DEFAULT),
      mLightCutoff(POINT_LIGHT_CUTOFF),
      mLightColor(COLOR_DEFAULT),
      mLightUniformsByShader(),
      mLightMesh(nullptr),
      mLightRenderer(nullptr)
{
    SCEScene::RegisterLight(SCEHandle<Light>(this));
    generateLightMesh();
    initRenderDataForShader(SCELighting::GetLightShader(), SCELighting::GetStencilShader());
    //change layer to avoid rendering the light mesh as a regular mesh
    container->SetLayer(LIGHTS_LAYER);
}


Light::~Light()
{
    SCEScene::UnregisterLight(SCEHandle<Light>(this));
}

void Light::initRenderDataForShader(GLuint lightShaderId, GLuint stencilShaderId)
{
    glUseProgram(lightShaderId);

    //get the location for all uniforms, some may not be used but that's ok.
    for(int type = LIGHT_POSITION; type < LIGHT_UNIFORMS_COUNT; ++type)
    {
        string name = s_lightUniformNames[type];
        GLuint uniform = glGetUniformLocation(lightShaderId, name.c_str());
        //set the uniformId for this uniform and shader if it's not done already
        if(mLightUniformsByShader[(LightUniformType)type].count(lightShaderId) == 0)
        {
            mLightUniformsByShader[(LightUniformType)type][lightShaderId] = uniform;
        }
    }

    mScreenSizeUniform = glGetUniformLocation(lightShaderId, SCREEN_SIZE_UNIFORM_NAME);

    mLightRenderer = GetContainer()->AddComponent<MeshRenderer>(lightShaderId);
    mLightStencilRenderer = GetContainer()->AddComponent<MeshRenderer>(stencilShaderId);
}

void Light::bindRenderDataForShader(GLuint shaderId)
{
    glUseProgram(shaderId);

    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    vec3 lightPos = transform->GetWorldPosition();
    vec4 tmp = transform->GetWorldTransform() * vec4(0, 0, 1, 0);
    vec3 lightDir = vec3(tmp.x, tmp.y, tmp.z);
    lightDir = glm::normalize(lightDir);

    //send the light data to the uniforms
    for(int type = LIGHT_POSITION; type < LIGHT_UNIFORMS_COUNT; ++ type)
    {
        GLuint unifId = mLightUniformsByShader[(LightUniformType)type][shaderId];

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
        case LIGHT_MAX_ANGLE :
            glUniform1f(unifId, mLightMaxAngle);
            break;
        case LIGHT_CUTOFF :
            glUniform1f(unifId, mLightCutoff);
            break;
        default :
            SCE::Debug::LogError("Unknown ligth uniform type : " + type);
        }
    }

    glUniform2f(mScreenSizeUniform, SCECore::GetWindowWidth(), SCECore::GetWindowHeight());
}

void Light::bindLightModelForShader(GLuint shaderId)
{
    glUseProgram(shaderId);

    //set subroutine according to light type
    GLuint shaderType = GL_FRAGMENT_SHADER; //always fragment shader for lighting computations (for now)

    //get the index of the subroutine uniform (form 0 to nb of subroutines uniforms)
    GLuint lightSubroutineUniform = glGetSubroutineUniformLocation(shaderId,
                                                                   shaderType,
                                                                   COMPUTE_LIGHT_UNIFORM_NAME);

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
        Debug::LogError("Unknown Light type : " + mLightType);
        break;
    }

    GLuint lightSubroutine = glGetSubroutineIndex( shaderId,
                                                   shaderType,
                                                   lightSubroutineName.c_str());

    //Get the number of active uniforms
    //This is expecting that subroutines are only used for light in this shader
    GLint subroutineUniformCount;
    glGetProgramStageiv(shaderId, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORMS, &subroutineUniformCount);

    if(subroutineUniformCount > 0)
    {
        Debug::Assert(subroutineUniformCount == LIGHT_ROUTINE_COUNT,
                      string("Wrong number of subroutine found in light pass shader \n")
                      + "expected " + std::to_string(LIGHT_ROUTINE_COUNT)
                      + " found " + std::to_string(subroutineUniformCount));

        //array of subroutine index for each subroutine uniform
        //ex : {indexForUniform1, indexForUniform2, indexForUniform3, ...}
        GLuint uniformArray[subroutineUniformCount];
        for(GLint uniformIndex = 0; uniformIndex < subroutineUniformCount; ++uniformIndex)
        {
            if(uniformIndex == (GLint)lightSubroutineUniform)
            {
                uniformArray[uniformIndex] = lightSubroutine;
            }
            else
            {
                Debug::LogError("Unexepected subroutine found at index : " + uniformIndex);
            }
        }

        //send the binding of uniform/subroutines to the current context/shader
        glUniformSubroutinesuiv( shaderType, subroutineUniformCount, uniformArray);
    }
}

const glm::vec4 &Light::GetLightColor() const
{
    return mLightColor;
}

void Light::SetLightColor(const glm::vec4 &lightColor)
{
    mLightColor = lightColor;
}

void Light::RenderDeffered(const SCEHandle<Camera>& camera)
{
    bindRenderDataForShader(SCELighting::GetLightShader());
    bindLightModelForShader(SCELighting::GetLightShader());

    mLightRenderer->Render(camera, mLightType == DIRECTIONAL_LIGHT);
}

void Light::RenderToStencil(const SCEHandle<Camera>& camera)
{
    mLightStencilRenderer->Render(camera, mLightType == DIRECTIONAL_LIGHT);
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
    if(mLightMesh)
    {
        GetContainer()->RemoveComponent<Mesh>(mLightMesh);
    }
    switch(mLightType)
    {
    case POINT_LIGHT:
        generatePointLightMesh();
        break;
    case DIRECTIONAL_LIGHT :
        generateDirectionalLightMesh();
        break;
    case SPOT_LIGHT :
        generateSpotLightMesh();
        break;
    }
    if(mLightRenderer)
    {
        mLightRenderer->UpdateRenderedMesh();
        mLightStencilRenderer->UpdateRenderedMesh();
    }
}

void Light::generateDirectionalLightMesh()
{
    SCEHandle<Container> container = GetContainer();
    mLightMesh = Mesh::AddQuadMesh(container, 2.0f, 2.0f);
}

void Light::generateSpotLightMesh()
{
    SCEHandle<Container> container = GetContainer();
    mLightMesh = Mesh::AddConeMesh(container, mLightReach, mLightMaxAngle, 4.0f);
}

void Light::generatePointLightMesh()
{
    //Compute radius at which light intensity goes under some threshold
//    float lightSphereRadius2 = sqrt(mLightReach / POINT_LIGHT_CUTOFF);

    //Compute radius at which light intensity goes under some threshold
    //using solution to quadratic equation 1/square(x/r + 1) = cutoff(c)
    //s = (-2*r + sqrt(4*r*r/c))/2
    float lightSphereRadius = 0.0f;
    float delta = (4.0f * mLightReach * mLightReach)/mLightCutoff;
    lightSphereRadius = (-2.0f * mLightReach  + sqrt(delta))/2.0f;

    SCEHandle<Container> container = GetContainer();
    mLightMesh = Mesh::AddSphereMesh(container, lightSphereRadius, 4.0f);
}
