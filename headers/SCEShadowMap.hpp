/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEShadowMap.hpp*********/
/**************************************/
#ifndef SCE_SCE_SHADOW_MAP_HPP
#define SCE_SCE_SHADOW_MAP_HPP

#include "SCEDefines.hpp"

namespace SCE
{
    class SCEShadowMap
    {
    public :

        SCEShadowMap();
        ~SCEShadowMap();

        bool        Init(GLuint shadowmapWidth, GLuint shadowmapHeight);
        void        BindForShadowPass();
        void        BindTextureToLightShader(GLuint textureUnit);

    private :

        GLuint      mFBOId;
        GLuint      mDepthTexture;
    };
}


#endif
