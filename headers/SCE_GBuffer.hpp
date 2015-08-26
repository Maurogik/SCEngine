/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCE_GBuffer.hpp*********/
/**************************************/

#ifndef SCE_G_BUFFER_HPP
#define SCE_G_BUFFER_HPP

#include "SCEDefines.hpp"

#define GBUFFER_TEXTURE_COUNT GBUFFER_NUM_TEXTURES

namespace SCE {

    class SCE_GBuffer {

    public :

        enum GBUFFER_TEXTURE_TYPE {
            GBUFFER_TEXTURE_TYPE_POSITION = 0,
            GBUFFER_TEXTURE_TYPE_DIFFUSE,
            GBUFFER_TEXTURE_TYPE_NORMAL,
            GBUFFER_NUM_TEXTURES
        };

                SCE_GBuffer();
                ~SCE_GBuffer();
        bool    Init(unsigned int windowWidth, unsigned int windowHeight);
        void    ClearFinalBuffer();
        void    BindForGeometryPass();
        void    BindForStencilPass();
        void    BindForLightPass();
        void    BindForShadowPass();
        void    EndShadowPass();
        void    BindForFinalPass();
        void    BindTexturesToLightShader();
        void    SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

    private :

        GLuint      mFBOId;
        GLuint      mTextures[GBUFFER_TEXTURE_COUNT];
        GLuint      mDepthTexture;
        GLuint      mFinalTexture;
    };
}

#endif
