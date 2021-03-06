#include "SunCycle.hpp"
#include "../headers/SCESkyRenderer.hpp"

using namespace SCE;
using namespace std;

SunCycle::SunCycle(SCE::SCEHandle<Container> container, float speed, vec3 axis)
    : GameObject(container, "SunCycle"), mSpeed(speed),
#ifdef AMBIANT_LIGHT
      mAmbiantStrength(0.08f),
#endif
      mRotationAxis(axis),
      mNoonSunColor(1.0, 1.0, 0.8, 0.7), mSunsetColor(0.35, 0.2, 0.0, 0.4),
      mMidnightSunColor(0.3, 0.3, 1.0, 0.001), mBaseFogColor(0.3, 0.6, 0.9),
      mBaseSkyTopColor(0.06, 0.4, 0.85), mBaseSkyBottomColor(0.65, 0.9, 1.0),
      mSunsetFogColor(0.8, 0.4, 0.2)
{
    mTransform =  container->AddComponent<Transform>();
    mLight = container->AddComponent<Light>(LightType::DIRECTIONAL_LIGHT);

    glm::vec3 sunPos = glm::vec3(10.0, 0.0, -40000.0);
    mTransform->SetScenePosition(sunPos);
    mTransform->LookAt(glm::vec3(0.0));

    mLight->SetLightColor(glm::vec4(1.0, 1.0, 0.8, 0.7));
    mLight->SetIsSunLight(true);

#ifdef SKY_LIGHT
    //create additional sky light to act as ambiant lighting
    SCEHandle<Container> skyLightCont = SCE::SCEScene::CreateContainer("SkyLightContainer");
    SCEHandle<Transform> skyLightTransform = skyLightCont->AddComponent<Transform>();
    mSkyLight = skyLightCont->AddComponent<Light>(LightType::DIRECTIONAL_LIGHT);
    skyLightTransform->SetScenePosition(glm::vec3(0.0, 2000.0, 0.0));
    skyLightTransform->SetSceneOrientation(glm::vec3(90.0, 0.0, 0.0));
#endif

#ifdef AMBIANT_LIGHT
    //create additional sky light to act as ambiant lighting
    SCEHandle<Container> ambLightCont = SCE::SCEScene::CreateContainer("AmbiantLightContainer");
    mAmbiantLightTransform = ambLightCont->AddComponent<Transform>();
    mAmbiantLight = ambLightCont->AddComponent<Light>(LightType::DIRECTIONAL_LIGHT);
#endif
}

void SunCycle::Update()
{
    float angle = float(SCE::Time::DeltaTime()) * mSpeed;
    glm::quat rot = glm::angleAxis(glm::radians(angle), mRotationAxis);
    glm::vec3 worldPos = mTransform->GetScenePosition();
    worldPos = rot * worldPos;
    mTransform->SetScenePosition(worldPos);
    mTransform->LookAt(glm::vec3(0.0));

    glm::vec3 sunDir = normalize(-worldPos);

    float noonStr = glm::dot(sunDir, glm::vec3(0.0, -1.0, 0.0));
    float midnightStr = glm::clamp(noonStr * -3.0f, 0.0f , 1.0f);
    noonStr = pow(glm::max(0.0f, noonStr), 2.0f);
    float sunsetStr = glm::dot(sunDir, glm::vec3(0.0, 0.0, 1.0));
    float sunsetPow = pow(abs(sunsetStr) + (1.0f - noonStr), 1.2f);

    glm::vec4 color = noonStr*mNoonSunColor + midnightStr*mMidnightSunColor
            + sunsetPow*mSunsetColor;
    color.a *= 1.0f - midnightStr;
    mLight->SetLightColor(color);

#ifdef AMBIANT_LIGHT
    glm::vec3 ambWorldPos = -1.0f*worldPos;
    ambWorldPos.y = -10.0f;
    mAmbiantLightTransform->SetScenePosition(ambWorldPos);
    mAmbiantLightTransform->LookAt(glm::vec3(0.0));
    mAmbiantLight->SetLightColor(color*mAmbiantStrength*(1.0f - midnightStr));
#endif

    //Color sky and fog for sunsets/sunraise/night
    glm::vec3 skyColorModifier = glm::mix(glm::vec3(1.0), glm::vec3(1.2, 0.4, 0.2),
                                          pow(abs(sunsetStr), 2.0f));
    skyColorModifier = glm::mix(skyColorModifier, vec3(0.001, 0.001, 0.002), midnightStr);

    glm::vec3 fogColor = mBaseFogColor*skyColorModifier;
    glm::vec3 skyTopColor = mBaseSkyTopColor*skyColorModifier;
    glm::vec3 skyBottomColor = mBaseSkyBottomColor*skyColorModifier + mBaseSkyTopColor*midnightStr*0.02f;

    SCE::SkyRenderer::SetSkyColors(skyBottomColor, skyTopColor, fogColor);

#ifdef SKY_LIGHT
    //Sky light hase the same color than the sky
    glm::vec3 skyAvgColor = glm::mix(skyTopColor, skyBottomColor, 0.7f);
    mSkyLight->SetLightColor(glm::vec4(skyAvgColor,
                                       glm::max(0.1f * dot(skyAvgColor, skyAvgColor), 0.008f)));
#endif
}
