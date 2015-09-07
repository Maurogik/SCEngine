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
#include "Mesh.hpp"
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
        SPOT_LIGHT
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
        std::map<GLuint, GLint>     mLightUniformsByShader[LIGHT_UNIFORMS_COUNT];
        SCEHandle<Mesh>             mLightMesh;
        SCEHandle<MeshRenderer>     mLightRenderer;

        void                        initRenderDataForShader(GLuint lightShaderId);
        void                        bindRenderDataForShader(GLuint shaderId, const vec3& cameraPosition);
        void                        bindLightModelForShader(GLuint shaderId);

        void                        generateLightMesh();
        void                        generateDirectionalLightMesh();
        void                        generateSpotLightMesh();
        void                        generatePointLightMesh();
    };

}


#endif
