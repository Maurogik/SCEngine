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

                    SCETexture(const std::string &filename, const std::string &samplerName);
                    SCETexture(const std::string &samplerName, int width, int height, glm::vec4 color, SCETextureFormat textureFormat = DDS_FORMAT, SCETextureWrap wrapMode = CLAMP_WRAP,  bool mipmapsOn = true);
                    ~SCETexture();
        GLuint      GetTextureId() const;
        void        BindTexture(GLuint textureUnit, GLuint samplerUniformId);

    private :

        GLuint              mTextureId;
        std::string         mName;
        int                 mWidth;
        int                 mHeight;

        /**
         * @brief load a texture from disk
         * @param filename
         */
        GLuint              loadTextureFromFile(const std::string &filename);

        GLuint              loadTexture(const std::string &filename, SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn);

        GLuint              createTexture(int width, int height, vec4 color, SCETextureFormat textureFormat, SCETextureWrap wrapMode,  bool mipmapsOn);

        SCETextureFormat    formatFromString(const std::string &formatString);

        SCETextureWrap      wrapModeFromString(const std::string &wrapString);

        uint                getSoilFlags(SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn);
    };

}

#endif
