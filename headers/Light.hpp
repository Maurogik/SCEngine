/********* Sand Castle Engine *********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.hpp **********/
/**************************************/
#ifndef SCE_LIGHT_HPP
#define SCE_LIGHT_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include "Camera.hpp"
#include "MeshRenderer.hpp"
#include "SCE_GBuffer.hpp"
#include <map>

namespace SCE
{

    enum LightUniformType
    {
        LIGHT_POSITION = 0,
        LIGHT_DIRECTION,
        LIGHT_REACH,
        LIGHT_COLOR,
        LIGHT_MAX_DOT,
        EYE_POSITION,
        SHADOW_STRENGTH,
        LIGHT_UNIFORMS_COUNT
    };

    enum LightType
    {
        DIRECTIONAL_LIGHT = 0,
        POINT_LIGHT,
        SPOT_LIGHT,
        LIGHT_TYPE_COUNT
    };

    class Light : public Component
    {
    public :

        virtual             ~Light();

        float               GetLightReach() const;
        void                SetLightReach(float lightReach);

        float               GetLightMaxAngle() const;
        void                SetLightMaxAngle(float lightMaxAngle);

        const glm::vec4&    GetLightColor() const;
        void                SetLightColor(const vec4 &lightColor);

        LightType           GetLightType() const;

        void                InitLightRenderData(GLuint lightShaderProgram);
        void                RenderWithLightData(const CameraRenderData& renderData);
        void                RenderWithoutLightData(const CameraRenderData& renderData);

        void                SetIsSunLight(bool isSunLight);

    protected :

        Light(SCEHandle<Container>& container, LightType GetLightType,
                                  const std::string& typeName = "");

    private :

        LightType                   mLightType;
        float                       mLightReach;
        float                       mLightMaxAngle;
        glm::vec4                   mLightColor;
        bool                        mIsSunLight;
        //array containing a map of uniforms Id by shader ID, for each light uniform type
        GLint                       mLightUniforms[LIGHT_UNIFORMS_COUNT];
        GLuint                      mLightSubroutineIndex;
        ui16                        mLightMeshId;
        SCEHandle<MeshRenderer>     mLightRenderer;

        void                        initRenderDataForShader(GLuint lightShaderId);
        void                        bindRenderDataForShader(const vec3& cameraPosition);
        void                        bindLightModelForShader();

        void                        generateLightMesh();
        ui16 generateDirectionalLightMesh();
        ui16 generateSpotLightMesh();
        ui16 generatePointLightMesh();
    };

}


#endif
