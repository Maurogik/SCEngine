/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.hpp*********/
/**************************************/


#ifndef SCE_LIGHTING_HPP
#define SCE_LIGHTING_HPP

#include "SCEDefines.hpp"
#include "SCE_GBuffer.hpp"

namespace SCE
{
    class SCELighting
    {
    public :

        SCELighting();

        static void         Init();
        static void         CleanUp();
        static void         StartLightRenderPass();
        static void         StartLightStencilPass();
        static GLuint       GetLightShader();
        static GLuint       GetStencilShader();
        static GLuint       GetTextureSamplerUniform(SCE_GBuffer::GBUFFER_TEXTURE_TYPE textureType);

    private :

        static SCELighting* s_instance;

        GLuint              mDefaultLightShader;
        GLuint              mLightStencilShader;
        std::string         mTexSamplerNames[SCE_GBuffer::GBUFFER_NUM_TEXTURES];
        GLuint              mTexSamplerUniforms[SCE_GBuffer::GBUFFER_NUM_TEXTURES];

        void         initLightShader();
    };
}

#endif
