/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETexture.hpp**********/
/**************************************/

#ifndef SCE_TEXTURE_H
#define SCE_TEXTURE_H

#include "SCEDefines.hpp"

namespace SCE {

    enum SCETextureFormat {
        DDS_FORMAT,
        UNCOMPRESSED_FORMAT
    };

    enum SCETextureWrap{
        CLAMP_WRAP,
        REPEAT_WRAP
    };

    class SCETexture {

    public:

                    SCETexture(const std::string &filename, const std::string &samplerName, GLuint shaderProgram);
        GLuint      GetTextureId() const;
        GLuint      GetSamplerUniformId() const;
        void        BindTexture(GLuint textureUnit);

    private :

        GLuint              mTextureId;
        GLuint              mSamplerUniformId;
        std::string         mName;

        /**
         * @brief load a texture from disk
         * TODO :
         * 1. cache texture so it can be reused later
         * 2. generate opengl texture
         * 3. bind texture data to opengl
         * 4. load parameter from description file
         * 5. apply parameters
         * @param filename
         */
        GLuint              loadTextureFromFile(const std::string &filename, GLuint shaderProgram);

        GLuint              loadTexture(const std::string &filename, GLuint shaderProgram, SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn);

        SCETextureFormat    formatFromString(const std::string &formatString);

        SCETextureWrap      wrapModeFromString(const std::string &wrapString);
    };

}

#endif
