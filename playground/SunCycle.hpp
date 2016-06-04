#ifndef SUN_CYCLE_HPP
#define SUN_CYCLE_HPP

#include "../headers/SCE.hpp"

#define SKY_LIGHT
#define AMBIANT_LIGHT

class SunCycle : public SCE::GameObject {

public :

    virtual void    Update();

protected :

                    SunCycle(SCE::SCEHandle<SCE::Container> container, float speed, glm::vec3 axis);

private :

    float mSpeed;
#ifdef AMBIANT_LIGHT
    float mAmbiantStrength;
#endif
    glm::vec3 mRotationAxis;
    glm::vec4 mNoonSunColor;
    glm::vec4 mSunsetColor;
    glm::vec4 mMidnightSunColor;
    glm::vec3 mBaseFogColor;
    glm::vec3 mBaseSkyTopColor;
    glm::vec3 mBaseSkyBottomColor;
    glm::vec3 mSunsetFogColor;
    SCE::SCEHandle<SCE::Light> mLight;
    SCE::SCEHandle<SCE::Light> mSkyLight;
    SCE::SCEHandle<SCE::Light> mAmbiantLight;
    SCE::SCEHandle<SCE::Transform> mAmbiantLightTransform;
    SCE::SCEHandle<SCE::Transform> mTransform;

};

#endif
