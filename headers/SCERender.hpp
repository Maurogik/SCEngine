/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCERender.hpp**********/
/**************************************/

#ifndef SCE_SCERENDER_HPP
#define SCE_SCERENDER_HPP

#include "SCE_GBuffer.hpp"
#include "SCEHandle.hpp"
#include "SCERenderStructs.hpp"
#include <vector>

namespace SCE
{
    class Camera;
    class Container;

    struct ToneMappingData
    {
        ToneMappingData()
            : toneMapShader(-1), luminanceShader(-1),
              exposureUniform(-1), maxBrightnessUniform(-1),
              exposure(1.0f), maxBrightness(1.5f) {}

        GLuint  toneMapShader;
        GLuint  luminanceShader;
        GLint   exposureUniform;
        GLint   maxBrightnessUniform;
        float   exposure;
        float   maxBrightness;
    };

    class SCERender
    {
    public :        

        static void         Init();
        static void         CleanUp();
        static void         Render(const SCEHandle<Camera> &camera,
                                   std::vector<Container*> objectsToRender);
        static void         ResetClearColorToDefault();
        static glm::mat4    FixOpenGLProjectionMatrix(const glm::mat4& projMat);
        static void         RenderFullScreenPass(GLuint shaderId,
                                                 const glm::mat4 &projectionMatrix,
                                                 const glm::mat4 &viewMatrix);

        ToneMappingData     mToneMapData;
        static SCERender*   s_instance;
    private :


        SCE_GBuffer         mGBuffer;
        glm::vec4           mDefaultClearColor;
        ui16                mQuadMeshId;

        SCERender();

        void                renderGeometryPass(const vec3& cameraPostion_worldspace, const CameraRenderData& renderData,
                                               std::vector<Container*> objectsToRender);

    };
}

#endif
