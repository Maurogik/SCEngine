/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.hpp*********/
/**************************************/


#ifndef SCE_LIGHTING_HPP
#define SCE_LIGHTING_HPP

#include "SCEDefines.hpp"
#include "SCEHandle.hpp"
#include "SCE_GBuffer.hpp"
#include "SCEShadowMap.hpp"
#include <vector>

namespace SCE
{
    class Camera;
    class Light;
    class Container;
    struct SCECameraData;

    class SCELighting
    {
    public :

        SCELighting();

        static void         Init();
        static void         CleanUp();

        static void         RenderShadowsToGBuffer(const SCECameraData& camRenderData,
                                                   std::vector<vec3> frustrumCorners,
                                                   std::vector<SCE::Container*> objectsToRender,
                                                   SCE::SCE_GBuffer& gBuffer);

        static void         RenderLightsToGBuffer(const SCECameraData& renderData,
                                                  SCE::SCE_GBuffer& gBuffer);

        static GLuint       GetLightShader();
        static GLuint       GetStencilShader();
        static GLuint       GetShadowMapShader();
        static GLuint       GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType);
        static GLuint       GetShadowmapSamplerUniform();
        static void         RegisterLight(SCEHandle<Light> light);
        static void         UnregisterLight(SCEHandle<Light> light);
        static void         SetShadowCaster(SCEHandle<Light> light);

    private :

        static SCELighting* s_instance;

        GLuint              mLightShader;
        GLuint              mEmptyShader;
        std::string         mTexSamplerNames[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLuint              mTexSamplerUniforms[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLuint              mShadowSamplerUnifom;
        GLuint              mShadowDepthMatUnifom;

        SCEShadowMap        mShadowMapFBO;

        SCEHandle<Light>                mShadowCaster;
        glm::mat4                       mDepthConvertMat;

        std::vector<SCEHandle<Light>>   mStenciledLights;
        std::vector<SCEHandle<Light>>   mDirectionalLights;

        void                initLightShader();
        void                registerLight(SCEHandle<Light> light);
        void                unregisterLight(SCEHandle<Light> light);

        void                renderLightingPass(const SCECameraData& renderData,
                                               SCEHandle<Light> &light);

        void                renderLightStencilPass(const SCECameraData& renderData,
                                                   SCEHandle<Light> &light);

        void                renderShadowmapPass(const SCECameraData& lightRenderData,
                                                std::vector<SCE::Container*> objectsToRender);
    };
}

#endif
