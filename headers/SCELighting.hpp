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
#include <vector>

namespace SCE
{
    //Forward declare Camera because we won't need the implementation
    class Camera;
    class Light;

    class SCELighting
    {
    public :

        SCELighting();

        static void         Init();
        static void         CleanUp();

        static void         RenderLightsToGBuffer(const SCEHandle<Camera>& camera,
                                                 SCE::SCE_GBuffer& gBuffer);
        static GLuint       GetLightShader();
        static GLuint       GetStencilShader();
        static GLuint       GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType);
        static void         RegisterLight(SCEHandle<Light> light);
        static void         UnregisterLight(SCEHandle<Light> light);

    private :

        static SCELighting* s_instance;

        GLuint              mLightShader;
        GLuint              mStencilShader;
        std::string         mTexSamplerNames[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLuint              mTexSamplerUniforms[SCE_GBuffer::GBUFFER_NUM_TEXTURES];

        std::vector<SCEHandle<Light>>   mStenciledLights;
        std::vector<SCEHandle<Light>>   mDirectionalLights;

        void                initLightShader();
        void                renderLightingPass(const SCEHandle<Camera>& camera, SCEHandle<Light> &light);
        void                renderLightStencilPass(const SCEHandle<Camera>& camera, SCEHandle<Light> &light);
        void                registerLight(SCEHandle<Light> light);
        void                unregisterLight(SCEHandle<Light> light);
    };
}

#endif
