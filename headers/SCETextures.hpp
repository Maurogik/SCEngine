/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETextures.hpp*********/
/**************************************/
#ifndef SCE_TEXTURES_HPP
#define SCE_TEXTURES_HPP

#include "SCEDefines.hpp"

namespace SCE
{


    namespace TextureUtils
    {
        enum SCETextureFormat
        {
            DDS_FORMAT,
            UNCOMPRESSED_FORMAT
        };

        enum SCETextureWrap
        {
            CLAMP_WRAP,
            REPEAT_WRAP
        };

        GLuint      CreateTexture(int width, int height,
                                          glm::vec4 color,
                                          SCETextureFormat textureFormat = DDS_FORMAT,
                                          SCETextureWrap wrapMode = REPEAT_WRAP,
                                          bool mipmapsOn = true);
        GLuint      LoadTexture(const std::string &textureName,
                                SCETextureFormat fallbackFormat = DDS_FORMAT,
                                SCETextureWrap fallbackWrapMode = REPEAT_WRAP,
                                bool fallbackMipmaps = true);
        void        DeleteTexture(GLuint textureId);
        void        BindTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId);
        //same but without debug
        void        BindSafeTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId);

#ifdef SCE_DEBUG_ENGINE
        void        EnableDebugTexture();
        void        DisableDebugTexture();
        bool        ToggleDebugTexture();
#endif
    }

}


#endif
