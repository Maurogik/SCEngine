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
#include "Mesh.hpp"
#include "MeshRenderer.hpp"

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
        DIRECTIONAL_LIGHT = 0,
        POINT_LIGHT,
        SPOT_LIGHT
    };

    class Light : public Component {

    public :

        virtual             ~Light();

        const float&        GetLightReach() const;
        void                SetLightReach(float lightReach);

        const float&        GetLightMaxAngle() const;
        void                SetLightMaxAngle(float lightMaxAngle);

        const glm::vec4&    GetLightColor() const;
        void                SetLightColor(glm::vec4 lightColor);

        void                RenderLight(const SCEHandle<Camera> &cam);

        static void         StartLightPass();
        static GLuint               s_DefaultLightShader;

    protected :

        Light(SCEHandle<Container>& container, const LightType &lightType,
                                  const std::string& typeName = "");

    private :


        LightType                   mLightType;
        float                       mLightReach;
        float                       mLightMaxAngle;
        glm::vec4                   mLightColor;
        //array containing a map of uniforms Id by shader ID, for each light uniform type
        std::map<GLuint, GLuint>    mLightUniformsByShader[LIGHT_UNIFORMS_COUNT];
        SCEHandle<Mesh>             mLightMesh;
        SCEHandle<MeshRenderer>     mLightRenderer;
        //tmp
        GLuint                      mScreenSizeUniform;

        void                        initLightShader();
        void                        initRenderDataForShader(const GLuint &shaderId);
        void                        bindRenderDataForShader(const GLuint &shaderId);
        void                        bindLightModelForShader(const GLuint &shaderId);

        void                        generateLightMesh();
        void                        generateDirectionalLightMesh();
        void                        generateSpotLightMesh();
        void                        generatePointLightMesh();
    };

}


#endif
