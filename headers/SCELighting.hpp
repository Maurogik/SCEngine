/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.hpp*********/
/**************************************/


#ifndef SCE_LIGHTING_HPP
#define SCE_LIGHTING_HPP

#include "SCEDefines.hpp"
#include "SCEHandle.hpp"
#include "../headers/SCE_GBuffer.hpp"

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
        static void         StartLightPass();
        static void         EndLightPass();
        static void         RenderLightToGBuffer(const SCEHandle<Camera>& camera,
                                                 SCEHandle<Light> &light,
                                                 SCE::SCE_GBuffer& gBuffer);
        static GLuint       GetLightShader();
        static GLuint       GetStencilShader();
        static GLuint       GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType);        

    private :

        static SCELighting* s_instance;

        GLuint              mLightShader;
        GLuint              mStencilShader;
        std::string         mTexSamplerNames[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLuint              mTexSamplerUniforms[SCE_GBuffer::GBUFFER_NUM_TEXTURES];

        void                initLightShader();
        void                renderLightingPass(const SCEHandle<Camera>& camera, SCEHandle<Light> &light);
        void                renderLightStencilPass(const SCEHandle<Camera>& camera, SCEHandle<Light> &light);
    };
}

#endif
