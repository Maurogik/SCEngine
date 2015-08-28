/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETextures.hpp*********/
/**************************************/
#ifndef SCE_TEXTURES_HPP
#define SCE_TEXTURES_HPP

#include "SCEDefines.hpp"
#include <map>
#include <vector>

namespace SCE
{

    enum SCETextureFormat {
        DDS_FORMAT,
        UNCOMPRESSED_FORMAT
    };

    enum SCETextureWrap{
        CLAMP_WRAP,
        REPEAT_WRAP
    };

    class SCETextures
    {

    public :

        SCETextures();
        ~SCETextures();

        static void         Init();
        static void         CleanUp();
        static GLuint       CreateTexture(int width, int height,
                                          glm::vec4 color,
                                          SCETextureFormat textureFormat = DDS_FORMAT,
                                          SCETextureWrap wrapMode = CLAMP_WRAP,
                                          bool mipmapsOn = true);
        static GLuint       LoadTexture(const std::string &textureName);
        static void         DeleteTexture(GLuint textureId);
        static void         BindTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId);

    private :

        static SCETextures*                 s_instance;

        std::map<std::string, GLuint>       mLoadedTextures;
        std::vector<GLuint>                 mCreatedTextures;

        GLuint              loadTextureFromMetadata(const std::string &filename);

        GLuint              loadTexture(const std::string &filename, SCETextureFormat format,
                                        SCETextureWrap wrapMode, bool mipmapsOn);

        GLuint              createTexture(int width, int height, vec4 color, SCETextureFormat textureFormat,
                                          SCETextureWrap wrapMode,  bool mipmapsOn);

        SCETextureFormat    formatFromString(const std::string &formatString);

        SCETextureWrap      wrapModeFromString(const std::string &wrapString);

        uint                getSoilFlags(SCETextureFormat format, SCETextureWrap wrapMode, bool mipmapsOn);
    };

}


#endif
