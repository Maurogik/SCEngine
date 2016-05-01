/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCETextRenderer.cpp*******/
/**************************************/

#include "../headers/SCETextRenderer.hpp"
#include "../headers/SCETools.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"
#include <stdio.h>


#define ATLAS_SIZE 512

namespace SCE
{

namespace TextRenderer
{

    namespace //anonymous namespace with translation unit local data
    {
        struct Font
        {
            std::vector<stbtt_packedchar> charData;
            uint firstCodepoint;
            uint nbChars;
            ui16 fontSize;
            ui16 atlasWidth;
            ui16 atlasHeight;
        };

        struct FontRenderData
        {
            FontRenderData() : texture(GL_INVALID_INDEX) {}
            GLuint verticesBuffer;
            GLuint uvBuffer;
            GLuint texture;
            GLuint vaoId;
        };

        std::vector<Font> fonts;
        std::vector<FontRenderData> fontsRenderData;

        void pushQuadVertices(std::vector<glm::vec3>& vertices, glm::vec3 currentPos,
                              float x0, float x1,
                              float y0, float y1)
        {
            glm::vec3 quadVerts[6] = {
                glm::vec3(x0, y0, 0.0),//bottom left
                glm::vec3(x1, y0, 0.0),//bottom right
                glm::vec3(x1, y1, 0.0),//top right
                glm::vec3(x0, y0, 0.0),//bottom left
                glm::vec3(x1, y1, 0.0),//top right
                glm::vec3(x0, y1, 0.0),//top left
            };

            for(glm::vec3 vert : quadVerts)
            {
                vertices.push_back(currentPos + vert);
            }
        }

        void pushQuadUvs(std::vector<glm::vec2>& uvs,
                         float s0, float s1,
                         float t0, float t1)
        {
            uvs.push_back(glm::vec2(s0, t0));//bottom left
            uvs.push_back(glm::vec2(s1, t0));//bottom right
            uvs.push_back(glm::vec2(s1, t1));//top right
            uvs.push_back(glm::vec2(s0, t0));//bottom left
            uvs.push_back(glm::vec2(s1, t1));//top right
            uvs.push_back(glm::vec2(s0, t1));//top left
        }
    }

    ui16 LoadFont(const std::string &fileName, ui16 fontSizeInPixel)
    {        
        std::string fullPath = RESSOURCE_PATH + fileName;
        FILE* fontFile = fopen(fullPath.c_str(), "rb");
        if(!fontFile)
        {
            fullPath = ENGINE_RESSOURCE_PATH + fileName;
            fontFile = fopen(fullPath.c_str(), "rb");
        }

        if(fontFile)
        {
            Font loadedFont;
            FontRenderData loadedRenderData;

            uint firstCodepoint = '!';
            uint lastCodepoint = 'z';
            int numchars = lastCodepoint - firstCodepoint + 1;

            //each character can take up to fontSize x fontSize pixels
            int atlasWidth = (numchars) * fontSizeInPixel; //leave a fontSize margin
            int atlasHeight = fontSizeInPixel;

            fseek(fontFile, 0, SEEK_END);
            unsigned long fileLength = ftell(fontFile);
            fseek(fontFile, 0, SEEK_SET);

            std::vector<unsigned char> ttfFileData(fileLength);
            std::vector<unsigned char> fontAtlasData(atlasWidth * atlasHeight);
            stbtt_pack_context atlasContext;

            if(fread(&ttfFileData[0], 1, fileLength, fontFile) != fileLength)
            {
                SCE::Debug::RaiseError("Error during font file loading, could no read whole file.");
            }
            //close file once we've read it
            fclose(fontFile);

            stbtt_fontinfo fontInfo;
            if(!stbtt_InitFont(&fontInfo, &ttfFileData[0], 0))
            {
                Debug::RaiseError("Could not load font : " + fullPath);
            }

            loadedFont.firstCodepoint = firstCodepoint;
            loadedFont.charData.resize(numchars);
            loadedFont.nbChars = numchars;
            loadedFont.fontSize = fontSizeInPixel;
            loadedFont.atlasWidth = atlasWidth;
            loadedFont.atlasHeight = atlasHeight;

            //pack rendered characters into a font atlas
            stbtt_PackBegin(&atlasContext, &fontAtlasData[0], atlasWidth, atlasHeight, 0, 1,  nullptr);
            //create atlas in bitmap
            int atlasRes = stbtt_PackFontRange(&atlasContext, &ttfFileData[0], 0, fontSizeInPixel,
                                               firstCodepoint, numchars, &loadedFont.charData[0]);
            if(!atlasRes)
            {
                Debug::RaiseError("Error occured while creating font atlas, may still work");
            }

            stbtt_PackEnd(&atlasContext);

            glGenTextures(1, &loadedRenderData.texture);
            glBindTexture(GL_TEXTURE_2D, loadedRenderData.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasWidth, atlasHeight, 0, GL_RED,
                         GL_UNSIGNED_BYTE, fontAtlasData.data());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            fonts.push_back(loadedFont);

            glGenBuffers(1, &loadedRenderData.verticesBuffer);
            glGenBuffers(1, &loadedRenderData.uvBuffer);

            glGenVertexArrays(1, &loadedRenderData.vaoId);

            fontsRenderData.push_back(loadedRenderData);
            return fonts.size() - 1;
        }
        else
        {
            Debug::RaiseError("Could not open font file : " + fullPath);
            return -1;
        }                
    }

    void UnloadFont(ui16 fontId)
    {
        glDeleteTextures(1, &fontsRenderData[fontId].texture);
        glDeleteBuffers(1, &fontsRenderData[fontId].verticesBuffer);
        glDeleteBuffers(1, &fontsRenderData[fontId].uvBuffer);

        fontsRenderData.erase(fontsRenderData.begin() + fontId);
        fonts.erase(fonts.begin() + fontId);
    }

    void RenderText(ui16 fontId, const std::string &text,
                    const glm::mat4& modelMatrix,
                    const glm::mat4& viewMatrix,
                    const glm::mat4& projectionMatrix,
                    GLuint vertexAttribLocation,
                    GLuint uvAttribLocation,
                    GLint fontAtlasUniform)
    {
        float pixelW = 1.0f / (float)fonts[fontId].atlasWidth;
        float pixelH = 1.0f / (float)fonts[fontId].atlasHeight;
        float invFontSize = 1.0f / float(fonts[fontId].fontSize);

        std::vector<glm::vec3> vertices;
        vertices.reserve(text.length() * 6);
        std::vector<glm::vec2> uvs;
        uvs.reserve(text.length() * 6);

        uint firstCodepoint = fonts[fontId].firstCodepoint;
        uint lastCodepoint = fonts[fontId].nbChars + fonts[fontId].firstCodepoint - 1;

        glm::vec3 posOffset = glm::vec3(0.0f);

        for(uint charInd = 0; charInd < text.length(); ++charInd)
        {
            uint codepoint = (uint)text[charInd];
            if(codepoint >= firstCodepoint && codepoint < lastCodepoint)
            {
                stbtt_packedchar charData = fonts[fontId].charData[codepoint - firstCodepoint];
                //stb_tt use top to bottom convention, so we need to invert the y axis for both
                //positions and uvs
                pushQuadVertices(vertices, posOffset,
                                 charData.xoff * invFontSize, charData.xoff2 * invFontSize,
                                 -charData.yoff2 * invFontSize, -charData.yoff * invFontSize);
                pushQuadUvs(uvs,
                            charData.x0 * pixelW, charData.x1 * pixelW,
                            charData.y1 * pixelH, charData.y0 * pixelH);

                posOffset.x += charData.xadvance * invFontSize;
            }
            else //unknown characters of space
            {
                posOffset.x += 0.3f;
            }
        }

        //Render text
        glBindBuffer(GL_ARRAY_BUFFER, fontsRenderData[fontId].verticesBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);


        glBindBuffer(GL_ARRAY_BUFFER, fontsRenderData[fontId].uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_DYNAMIC_DRAW);



        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fontsRenderData[fontId].texture);
        glUniform1i(fontAtlasUniform, 0);


        glBindVertexArray(fontsRenderData[fontId].vaoId);

        glEnableVertexAttribArray(uvAttribLocation);
        glBindBuffer(GL_ARRAY_BUFFER, fontsRenderData[fontId].uvBuffer);
        glVertexAttribPointer(uvAttribLocation,
                              2, //nb float per comp
                              GL_FLOAT,
                              GL_FALSE, //normalized ?
                              0, 0);

        glEnableVertexAttribArray(vertexAttribLocation);
        glBindBuffer(GL_ARRAY_BUFFER, fontsRenderData[fontId].verticesBuffer);
        glVertexAttribPointer(vertexAttribLocation,
                              3, //nb float per comp
                              GL_FLOAT,
                              GL_FALSE, //normalized ?
                              0, 0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glDisableVertexAttribArray(vertexAttribLocation);
        glDisableVertexAttribArray(uvAttribLocation);

        glBindVertexArray(0);
    }

}

}
