/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEDebugText.cpp*********/
/**************************************/


#include "../headers/SCEDebugText.hpp"
#include "../headers/SCETextRenderer.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCEShaders.hpp"

#define FONT_SIZE 20

#ifdef SCE_DEBUG_ENGINE

namespace SCE
{

namespace DebugText
{

    namespace
    {
        struct DebugTextData
        {
            DebugTextData() : shaderProgram(GL_INVALID_INDEX) {}
            GLuint  shaderProgram;
            GLuint  vertexAttribLoc;
            GLuint  uvAttribLoc;
            GLint   textColorUniform;
            GLint   fontAtlasUniform;
            ui16    fontId;
        };

        struct DebugLogEntry
        {
            std::string message;
            glm::vec3   color;
        };

        struct DebugStringEntry
        {
            std::string text;
            glm::vec3   color;
            glm::vec2   position;
        };


        DebugTextData debugTextRenderData;
        std::vector<DebugLogEntry> debugMessages;
        std::vector<DebugStringEntry> debugStrings;
        glm::vec3 defaultTextColor;

        void initializeDebugTextRenderData()
        {
            debugTextRenderData.fontId =
                    SCE::TextRenderer::LoadFont("Fonts/open-sans/OpenSans-Regular.ttf", FONT_SIZE);
        //    debugTextRenderData.fontId = SCE::TextRenderer::LoadFont("Fonts/arial.ttf", FONT_SIZE);
            debugTextRenderData.shaderProgram = SCE::ShaderUtils::CreateShaderProgram("TextDebug");

            debugTextRenderData.vertexAttribLoc =
                    glGetAttribLocation(debugTextRenderData.shaderProgram, "vertexPosition_modelspace");
            debugTextRenderData.uvAttribLoc =
                    glGetAttribLocation(debugTextRenderData.shaderProgram, "vertexUV");

            debugTextRenderData.textColorUniform =
                    glGetUniformLocation(debugTextRenderData.shaderProgram, "TextColor");
            debugTextRenderData.fontAtlasUniform =
                    glGetUniformLocation(debugTextRenderData.shaderProgram, "FontAtlas");
        }

    }

    void LogMessage(const std::string &message)
    {
        LogMessage(message, defaultTextColor);
    }

    void LogMessage(const std::string &message, const vec3& color)
    {
        DebugLogEntry entry;
        entry.color = color;
        entry.message = message;

        debugMessages.push_back(entry);
    }

    void RenderMessages(const glm::mat4& viewMatrix,
                        const glm::mat4& projectionMatrix)
    {
        if(debugTextRenderData.shaderProgram == GL_INVALID_INDEX)
        {
            initializeDebugTextRenderData();
        }

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(debugTextRenderData.shaderProgram);

        uint width = SCECore::GetWindowWidth();
        uint height = SCECore::GetWindowHeight();
        float fontHeight = float(FONT_SIZE) / float(height);
        float fontWidth = float(FONT_SIZE) / float(width);

        glm::mat4 textModelMatrix = glm::inverse(projectionMatrix * viewMatrix);
        glm::mat4 messagesModelMatrix = textModelMatrix;
        //move to top left (else 0.0, 0.0 is screen center) and restore fullscreen scale
        messagesModelMatrix = glm::translate(messagesModelMatrix, glm::vec3(-1.0, 1.0, 0.0));
        messagesModelMatrix = glm::scale(messagesModelMatrix, glm::vec3(2.0));
        //scale to font size so 1.0 is on character in size
        messagesModelMatrix = glm::scale(messagesModelMatrix, glm::vec3(fontWidth, fontHeight, 0.0));
        //leave on char of space with screen edges
        messagesModelMatrix = glm::translate(messagesModelMatrix, glm::vec3(1.0, -1.0, 0.0));

        //Render messages
        for(DebugLogEntry& entry : debugMessages)
        {
            SCE::ShaderUtils::BindDefaultUniforms(debugTextRenderData.shaderProgram,
                                                  messagesModelMatrix, viewMatrix, projectionMatrix);

            //bind text colo uniform
            glUniform3fv(debugTextRenderData.textColorUniform, 1, &(entry.color[0]));
            SCE::TextRenderer::RenderText(debugTextRenderData.fontId, entry.message,
                                          messagesModelMatrix, viewMatrix, projectionMatrix,
                                          debugTextRenderData.vertexAttribLoc,
                                          debugTextRenderData.uvAttribLoc,
                                          debugTextRenderData.fontAtlasUniform);

            messagesModelMatrix = glm::translate(messagesModelMatrix, glm::vec3(0.0, -1.0, 0.0));
        }

        //move origin to bottom left corner
        textModelMatrix = glm::translate(textModelMatrix, glm::vec3(-1.0, -1.0, 0.0));
        textModelMatrix = glm::scale(textModelMatrix, glm::vec3(2.0));
        //Render strings
        for(DebugStringEntry& entry : debugStrings)
        {
            glm::vec3 pos = glm::vec3(entry.position.x, entry.position.y, 0.0f);
            glm::mat4 stringModelMatrix = glm::translate(textModelMatrix, pos);
            //set the font size by scaling the model matrix
            stringModelMatrix = glm::scale(stringModelMatrix, glm::vec3(fontWidth, fontHeight, 0.0));

            SCE::ShaderUtils::BindDefaultUniforms(debugTextRenderData.shaderProgram,
                                                  stringModelMatrix, viewMatrix, projectionMatrix);

            //bind text colo uniform
            glUniform3fv(debugTextRenderData.textColorUniform, 1, &(entry.color[0]));
            SCE::TextRenderer::RenderText(debugTextRenderData.fontId, entry.text,
                                          stringModelMatrix, viewMatrix, projectionMatrix,
                                          debugTextRenderData.vertexAttribLoc,
                                          debugTextRenderData.uvAttribLoc,
                                          debugTextRenderData.fontAtlasUniform);
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        //clear this frame entries
        int nbEntries = debugMessages.size();
        debugMessages.clear();
        debugMessages.reserve(nbEntries);

        nbEntries = debugStrings.size();
        debugStrings.clear();
        debugStrings.reserve(nbEntries);
    }

    void SetDefaultPrintColor(const vec3 &color)
    {
        defaultTextColor = color;
    }

    void RenderString(const vec2 &position, std::string const& text, const vec3 &color)
    {
        DebugStringEntry entry;
        entry.color = color;
        entry.text = text;
        entry.position = position;

        debugStrings.push_back(entry);
    }

    void RenderString(const vec2 &position, const std::string &text)
    {
        RenderString(position, text, defaultTextColor);
    }

}

}

#endif
