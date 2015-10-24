/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEDebugText.cpp*********/
/**************************************/


#include "../headers/SCEDebugText.hpp"
#include "../headers/SCETextRenderer.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCEShaders.hpp"

#define FONT_SIZE 60

namespace SCE
{

namespace DebugText
{

    namespace
    {

        struct DebugTextData
        {
            DebugTextData() : shaderProgram(-1) {}
            GLuint  shaderProgram;
            GLuint  vertexAttribLoc;
            GLuint  uvAttribLoc;
            GLint   textColorUniform;
            GLint   fontAtlasUniform;
            ui16    fontId;
        };

        struct DebugTextEntry
        {
            std::string message;
            glm::vec3   color;
        };

        DebugTextData debugTextRenderData;
        std::vector<DebugTextEntry> debugMessages;

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

    void Print(const std::string &message)
    {
        Print(message, glm::vec3(0.0, 0.0, 0.0));
    }

    void Print(const std::string &message, const vec3& color)
    {
        DebugTextEntry entry;
        entry.color = color;
        entry.message = message;

        debugMessages.push_back(entry);
    }

    void RenderMessages(const glm::mat4& viewMatrix,
                        const glm::mat4& projectionMatrix)
    {
        if(debugTextRenderData.shaderProgram == GLuint(-1))
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
        textModelMatrix = glm::translate(textModelMatrix, glm::vec3(-1.0, 1.0, 0.0));
        textModelMatrix = glm::scale(textModelMatrix, glm::vec3(fontWidth, fontHeight, 0.0));
        textModelMatrix = glm::translate(textModelMatrix, glm::vec3(1.0, -1.0, 0.0));


        //Render messages
        for(DebugTextEntry& entry : debugMessages)
        {
            SCE::ShaderUtils::BindDefaultUniforms(debugTextRenderData.shaderProgram,
                                                  textModelMatrix, viewMatrix, projectionMatrix);

            //bind text colo uniform
            glUniform3fv(debugTextRenderData.textColorUniform, 1, &(entry.color[0]));
            SCE::TextRenderer::RenderText(debugTextRenderData.fontId, entry.message,
                                          textModelMatrix, viewMatrix, projectionMatrix,
                                          debugTextRenderData.vertexAttribLoc,
                                          debugTextRenderData.uvAttribLoc,
                                          debugTextRenderData.fontAtlasUniform);

            textModelMatrix = glm::translate(textModelMatrix, glm::vec3(0.0, -1.0, 0.0));
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        //clear this frame entries
        int nbEntries = debugMessages.size();
        debugMessages.clear();
        debugMessages.reserve(nbEntries);
    }

}

}
