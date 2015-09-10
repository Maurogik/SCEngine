/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.cpp**********/
/**************************************/


#include "../headers/SCETerrain.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCERenderStructs.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>


#define TERRAIN_SHADER_NAME "TerrainTess"
#define TERRAIN_TEXTURE_NAME "TerrainHeightMap"
#define QUAD_TO_TERRAIN_UNIFORM_NAME "QuadToTerrainSpace"
#define TERRAIN_TO_WORLD_UNIFORM_NAME "TerrainToWorldspace"
#define WORLD_TO_VIEW_UNITFORM_NAME "V"
#define PROJECTION_UNIFORM_NAME "P"
#define TERRAIN_MVP_UNIFORM_NAME "MVP"
#define TERRAIN_MV_UNIFORM_NAME "MV"
#define TERRAIN_TEXTURE_SIZE 1024

namespace SCE
{

namespace Terrain
{
    struct TerrainQuadUniforms
    {
        GLint quadToTerrainMatrix;
        GLint terrainToWorldMatrix;
        GLint viewMatrix;
        GLint projectionMatrix;
        GLint MVPMatrix;
        GLint MVMatrix;
    };

    struct TerrainGLData
    {
        GLuint terrainProgram;
        GLuint terrainTexture;
        GLint terrainTextureUniform;
    };

    static glm::vec3 quadVertices[4] =
    {
        glm::vec3(-0.5, 0.0, -0.5),
        glm::vec3(0.5, 0.0, -0.5),
        glm::vec3(0.5, 0.0, 0.5),
        glm::vec3(-0.5, 0.0, 0.5)
    };
    static uint8_t quadPatchIndices[] = {0, 1, 2, 3}; //indices for a quad, not 2 triangles


    //Terrain quad render data
    static GLuint  quadVao;
    static GLuint  quadVerticesVbo;
    static GLuint  quadIndicesVbo;

    static TerrainGLData terrainGLData;
    static TerrainQuadUniforms terrainQuadUniforms;

    void cleanup()
    {
        if(terrainGLData.terrainTexture != GLuint(-1))
        {
            glDeleteTextures(1, &(terrainGLData.terrainTexture));
        }

        if(terrainGLData.terrainProgram != GLuint(-1))
        {
            SCEShaders::DeleteShaderProgram(terrainGLData.terrainProgram);
        }
    }

    //generate perlin noise texture
    void initializeTerrainTexture()
    {
        //8MB array, does not fit on stack so heap allocate it
        float *pixels = new float[TERRAIN_TEXTURE_SIZE * TERRAIN_TEXTURE_SIZE];

        float x, y = 0.1f, z;

        for(int xCount = 0; xCount < TERRAIN_TEXTURE_SIZE; ++xCount)
        {
            x = float(xCount) / float(TERRAIN_TEXTURE_SIZE);
            for(int zCount = 0; zCount < TERRAIN_TEXTURE_SIZE; ++zCount)
            {
                z = float(zCount) / float(TERRAIN_TEXTURE_SIZE);
                float noise = stb_perlin_noise3(x, y, z);
                pixels[xCount * TERRAIN_TEXTURE_SIZE + zCount] = noise;
            }
        }

        glGenTextures(1, &(terrainGLData.terrainTexture));
        glBindTexture(GL_TEXTURE_2D, terrainGLData.terrainTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, TERRAIN_TEXTURE_SIZE, TERRAIN_TEXTURE_SIZE, 0,
                     GL_RED, GL_FLOAT, pixels);

        delete[] pixels;
    }

    void initializeRenderData()
    {
        terrainGLData.terrainProgram = SCEShaders::CreateShaderProgram(TERRAIN_SHADER_NAME);

        GLuint terrainProgram = terrainGLData.terrainProgram;
        terrainGLData.terrainTextureUniform = glGetUniformLocation(terrainProgram, TERRAIN_TEXTURE_NAME);
        terrainQuadUniforms.quadToTerrainMatrix = glGetUniformLocation(terrainProgram, QUAD_TO_TERRAIN_UNIFORM_NAME);
        terrainQuadUniforms.terrainToWorldMatrix = glGetUniformLocation(terrainProgram, TERRAIN_TO_WORLD_UNIFORM_NAME);
        terrainQuadUniforms.viewMatrix = glGetUniformLocation(terrainProgram, WORLD_TO_VIEW_UNITFORM_NAME);
        terrainQuadUniforms.projectionMatrix = glGetUniformLocation(terrainProgram, PROJECTION_UNIFORM_NAME);
        terrainQuadUniforms.MVPMatrix = glGetUniformLocation(terrainProgram, TERRAIN_MVP_UNIFORM_NAME);
        terrainQuadUniforms.MVMatrix = glGetUniformLocation(terrainProgram, TERRAIN_MV_UNIFORM_NAME);

        //vertices buffer
        glGenBuffers(1, &quadVerticesVbo);
        glBindBuffer(GL_ARRAY_BUFFER, quadVerticesVbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //indices buffer
        glGenBuffers(1, &quadIndicesVbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndicesVbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(uint8_t), quadPatchIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        //quad VAO creation
        glGenVertexArrays(1, &quadVao);
        glBindVertexArray(quadVao);

        //setup VAO operations for automagic reuse
        glBindBuffer(GL_ARRAY_BUFFER, quadVerticesVbo);
        GLuint vertexAttribLocation = glGetAttribLocation(terrainProgram, "vertexPosition_modelspace");
        glEnableVertexAttribArray(vertexAttribLocation);
        glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
    }

    void renderPatch(const glm::mat4& projectionMatrix,
                     const glm::mat4& viewMatrix,
                     const glm::mat4& terrainToWorldspace,
                     const glm::vec3& position_terrainSpace,
                     float patchSize)
    {
        //create matrix to convert vertex pos to normalized terrain pos
        glm::mat4 quadToTerrainspace = glm::translate(mat4(1.0f), position_terrainSpace);
        quadToTerrainspace = quadToTerrainspace * glm::scale(mat4(1.0f), glm::vec3(patchSize));
        //Model to world matrix is concatenation of quad an terrain matrices
        glm::mat4 modelMatrix = terrainToWorldspace * quadToTerrainspace;
        glm::mat4 MVMat =  viewMatrix * modelMatrix;
        glm::mat4 MVPMat = projectionMatrix * MVMat;

        //do the rendering
        glUniformMatrix4fv(terrainQuadUniforms.quadToTerrainMatrix, 1, GL_FALSE, &(quadToTerrainspace[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.terrainToWorldMatrix,1, GL_FALSE, &(terrainToWorldspace[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.viewMatrix,          1, GL_FALSE, &(viewMatrix[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.projectionMatrix,    1, GL_FALSE, &(quadToTerrainspace[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.MVPMatrix,           1, GL_FALSE, &(MVPMat[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.MVMatrix,            1, GL_FALSE, &(MVMat[0][0]));

        glDrawElements(GL_PATCHES,
                       4,
                       GL_UNSIGNED_SHORT,
                       0);
    }

    void RenderTerrain(const glm::mat4& projectionMatrix,
                       const glm::mat4& viewMatrix,
                       float terrainBaseHeight)
    {
        float terrainSize = 200.0f;
        float halfTerrainSize = terrainSize / 2.0f;
        float patchSize = 20.0f;
        float halfPatchSize = patchSize / 2.0f;

        //setup gl state that at common for all patches
        glUseProgram(terrainGLData.terrainProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainGLData.terrainTexture);
        glUniform1i(terrainGLData.terrainTextureUniform, 0);//terrain height map is sampler 0
        glBindVertexArray(quadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndicesVbo);

        glm::vec3 cameraPosition_worldspace = glm::vec3(-viewMatrix[3]);//extract and invert translation collumn
        glm::vec3 terrainPosition_worldspace = cameraPosition_worldspace;
        terrainPosition_worldspace.y = terrainBaseHeight;

        //terrainspace (normalized from -1 to 1) to worldspace
        glm::mat4 terrainToWorldspace = glm::translate(glm::mat4(1.0f), terrainPosition_worldspace) *
                                        glm::scale(mat4(1.0), glm::vec3(halfTerrainSize));

        //Render all the terrain patches
        for(float x = -halfTerrainSize; x < halfTerrainSize; x += patchSize)
        {
            for(float z = -halfTerrainSize; z < halfTerrainSize; z += patchSize)
            {
                glm::vec3 pos_terrainspace((x + halfPatchSize)/halfTerrainSize,
                                           0.0f,
                                           (z + halfPatchSize)/halfTerrainSize);
                renderPatch(projectionMatrix,
                            viewMatrix,
                            terrainToWorldspace,
                            pos_terrainspace,
                            patchSize/halfTerrainSize);
            }
        }

        glBindVertexArray(0);
    }

    void Init()
    {
        initializeRenderData();
        initializeTerrainTexture();
    }

    void Cleanup()
    {
        cleanup();
    }
}

}
