/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.hpp **********/
/**************************************/
#ifndef SCE_LIGHT_HPP
#define SCE_LIGHT_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include <map>

namespace SCE {

    enum LightUniformType {
        LIGHT_POSITION = 0,
        LIGHT_DIRECTION,
        LIGHT_REACH,
        LIGHT_COLOR,
        LIGHT_START_RADIUS,
        LIGHT_END_RADIUS,
        LIGHT_UNIFORMS_COUNT
    };

    enum LightType{
        DIRECTIONAL_LIGHT,
        POINT_LIGHT,
        SPOT_LIGHT
    };

    class Light : public Component {

    public :

        virtual             ~Light();

        void                InitRenderDataForShader(const GLuint &shaderId);

        void                BindRenderDataForShader(const GLuint &shaderId);

        float GetLightReach() const;
        void SetLightReach(float lightReach);

        float GetLightStartRadius() const;
        void SetLightStartRadius(float lightStartRadius);

        float GetLightEndRadius() const;
        void SetLightEndRadius(float lightEndRadius);

        float GetLightColor() const;
        void SetLightColor(float lightColor);

    protected :

        Light(SCEHandle<Container>& container, const LightType &lightType,
                                  const std::string& typeName = "");

    private :

        LightingType                mLightingType;
        LightType                   mLightType;
        float                       mLightReach;
        float                       mLightStartRadius;
        float                       mLightEndRadius;
        float                       mLightColor;
        std::map<GLuint, GLuint>    mLightUniformsByShader[LIGHT_UNIFORMS_COUNT];

    };

}


#endif
