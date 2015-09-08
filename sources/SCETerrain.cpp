/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.cpp**********/
/**************************************/


#include "../headers/SCETerrain.hpp"
#include "../headers/SCEShaders.hpp"

#define TERRAIN_SHADER_NAME "TerrainShader"


namespace SCE
{

namespace Terrain
{
    static glm::vec3 quadVertices[4] =
    {
        glm::vec3(-0.5, 0.0, -0.5),
        glm::vec3(0.5, 0.0, -0.5),
        glm::vec3(0.5, 0.0, 0.5),
        glm::vec3(-0.5, 0.0, 0.5)
    };
    static uint8_t quadPatchIndices[] = {0, 1, 2, 3}; //indices for a quad, not 2 triangles

    static GLuint terrainProgram = -1;
    static glm::mat4 worldToTerrainSpace;

    //Terrain render data
    static GLuint  quadVao;
    static GLuint  quadVerticesVbo;
    static GLuint  quadIndicesVbo;

    void initializeTerrainShader()
    {
        terrainProgram = SCEShaders::CreateShaderProgram(TERRAIN_SHADER_NAME);
    }

    void initializeRenderData()
    {
        // Setup data
        glGenBuffers(1, &quadVerticesVbo);
        glBindBuffer(GL_ARRAY_BUFFER, quadVerticesVbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &quadIndicesVbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndicesVbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(uint8_t), quadPatchIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &quadVao);
        glBindVertexArray(quadVao);
        glBindBuffer(GL_ARRAY_BUFFER, quadVao);

        GLuint vertexAttribLocation = glGetAttribLocation(terrainProgram, "vertexPosition_modelspace");


        glEnableVertexAttribArray(vertexAttribLocation);
        glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
    }

    void InitTerrain()
    {

    }

    void RenderTerrain()
    {

    }
}

}
