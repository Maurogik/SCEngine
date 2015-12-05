/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.hpp*********/
/**************************************/


#ifndef SCE_LIGHTING_HPP
#define SCE_LIGHTING_HPP

#include "SCEDefines.hpp"
#include "SCERenderStructs.hpp"
#include "SCEHandle.hpp"
#include "SCE_GBuffer.hpp"
#include "SCEShadowMap.hpp"
#include <vector>

namespace SCE
{
    class Camera;
    class Light;
    class Container;
    class MeshRenderer;

    class SCELighting
    {
    public :

        static void         Init();
        static void         CleanUp();

        static void         RenderCascadedShadowMap(const CameraRenderData& camRenderData,
                                                   FrustrumData camFrustrumData,
                                                   mat4 camToWorldMat,
                                                   std::vector<SCE::MeshRenderer *> &objectsToRender);

        static void         RenderLightsToGBuffer(const CameraRenderData& renderData,
                                                  SCE::SCE_GBuffer& gBuffer);

        static void         RenderSkyToGBuffer(const CameraRenderData& renderData,
                                                  SCE::SCE_GBuffer& gBuffer);

        static GLuint       GetLightShader();
        static GLuint       GetStencilShader();
        static GLuint       GetShadowMapShader();
        static GLuint       GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType);
        static GLuint       GetShadowmapSamplerUniform();
        static void         RegisterLight(SCEHandle<Light> light);
        static void         UnregisterLight(SCEHandle<Light> light);
        static void         SetSunLight(SCEHandle<Light> light);

    private :

        static SCELighting* s_instance;

        GLuint              mLightShader;
        GLuint              mEmptyShader;
        std::string         mTexSamplerNames[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLint               mTexSamplerUniforms[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLint               mShadowSamplerUnifom;
        GLint               mShadowDepthMatUnifom;
        GLint               mShadowFarSplitUnifom;   

        SCEShadowMap        mShadowMapFBO;

        SCEHandle<Light>                mMainLight;
        std::vector<glm::mat4>          mDepthConvertMatrices;
        std::vector<float>              mFarSplit_cameraspace;

        std::vector<SCEHandle<Light>>   mStenciledLights;
        std::vector<SCEHandle<Light>>   mDirectionalLights;

        SCELighting();

        void                initLightShader();
        void                registerLight(SCEHandle<Light> light);
        void                unregisterLight(SCEHandle<Light> light);

        void                renderLightingPass(const CameraRenderData& renderData,
                                               SCEHandle<Light> &light);

        void                renderLightStencilPass(const CameraRenderData& renderData,
                                                   SCEHandle<Light> &light);

        void                renderShadowmapPass(const CameraRenderData& lightRenderData,
                                                std::vector<SCE::MeshRenderer *> &objectsToRender,
                                                uint shadowmapId);

        std::vector<CameraRenderData> computeCascadedLightFrustrums(FrustrumData cameraFrustrum,
                                                                    mat4 camToWorldMat);
    };
}

#endif
