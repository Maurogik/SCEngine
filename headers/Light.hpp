/********* Sand Castle Engine *********/
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
        LIGHT_MAX_ANGLE,
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
        void                BindLightModelForShader(const GLuint &shaderId);

        const float&        GetLightReach() const;
        void                SetLightReach(float lightReach);

        const float&        GetLightMaxAngle() const;
        void                SetLightMaxAngle(float lightMaxAngle);

        const glm::vec4&    GetLightColor() const;
        void                SetLightColor(glm::vec4 lightColor);

    protected :

        Light(SCEHandle<Container>& container, const LightType &lightType,
                                  const std::string& typeName = "");

    private :

//        LightingType                mLightingType;
        LightType                   mLightType;
        float                       mLightReach;
        float                       mLightMaxAngle;
        glm::vec4                   mLightColor;
        //array containing a map of uniforms Id by shader ID, for each light uniform type
        std::map<GLuint, GLuint>    mLightUniformsByShader[LIGHT_UNIFORMS_COUNT];

    };

}


#endif
