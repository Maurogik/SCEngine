/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEShadowMap.hpp*********/
/**************************************/
#ifndef SCE_SHADOW_MAP_HPP
#define SCE_SHADOW_MAP_HPP

#include "SCEDefines.hpp"

namespace SCE
{
    class SCEShadowMap
    {
    public :

        SCEShadowMap();
        ~SCEShadowMap();

        bool        Init(GLuint shadowmapWidth, GLuint shadowmapHeight, GLuint cascadeCount);
        void        BindForShadowPass(GLuint cascadeId);
        void        BindTextureToLightShader(GLuint textureUnit);

    private :

        GLuint      mFBOId;
        GLuint      mDepthTexture;
    };
}


#endif
