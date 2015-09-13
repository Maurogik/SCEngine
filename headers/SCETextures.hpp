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

        GLuint       CreateTexture(int width, int height,
                                          glm::vec4 color,
                                          SCETextureFormat textureFormat = DDS_FORMAT,
                                          SCETextureWrap wrapMode = CLAMP_WRAP,
                                          bool mipmapsOn = true);
        GLuint       LoadTexture(const std::string &textureName);
        void         DeleteTexture(GLuint textureId);
        void         BindTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId);
    }

}


#endif
