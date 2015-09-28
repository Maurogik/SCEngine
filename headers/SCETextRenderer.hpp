/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCETextRenderer.hpp*******/
/**************************************/
#ifndef SCE_TEXTRENDERER_HPP
#define SCE_TEXTRENDERER_HPP

#include "SCEDefines.hpp"
#include "stb_truetype.h"

namespace SCE
{

    namespace TextRenderer
    {
        ui16 LoadFont(const std::string& fileName, ui16 fontSizeInPixel);
        void UnloadFont(ui16 fontId);
        void RenderText(ui16 fontId, const std::string &text,
                        const mat4 &modelMatrix,
                        const mat4 &viewMatrix,
                        const mat4 &projectionMatrix, GLuint vertexAttribLocation, GLuint uvAttribLocation, GLint fontAtlasUniform);
    }

}

#endif
