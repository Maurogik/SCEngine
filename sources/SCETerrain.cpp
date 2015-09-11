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
#define TERRAIN_MODEL_UNIFORM_NAME "M"
#define TERRAIN_VIEW_UNIFORM_NAME "V"
#define TERRAIN_PROJECTION_UNIFORM_NAME "P"
#define TERRAIN_MVP_UNIFORM_NAME "MVP"

#define TERRAIN_TEXTURE_SIZE 1024

namespace SCE
{

namespace Terrain
{
    struct TerrainQuadUniforms
    {
        GLint quadToTerrainMatrix;
        GLint modelMatrix;
        GLint MVPMatrix;
    };

    struct TerrainGLData
    {
        GLuint terrainProgram;
        GLuint terrainTexture;
        GLint terrainTextureUniform;
        GLint viewMatrixUniform;
        GLint projectionMatrixUniform;
    };

    static glm::vec3 quadVertices[4] =
    {
        glm::vec3(-0.5f, 0.0f, -0.5f),
        glm::vec3(0.5f, 0.0f, -0.5f),
        glm::vec3(0.5f, 0.0f, 0.5f),
        glm::vec3(-0.5f, 0.0f, 0.5f)
    };
//    static ushort quadPatchIndices[] = {0, 1, 2, 3, 0, 2}; //indices for a quad, not 2 triangles
    static ushort quadPatchIndices[] = {0, 1, 2, 3}; //indices for a quad, not 2 triangles


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

    float lerp(float a, float b, float v)
    {
        return a + (b - a) * v;
    }

    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = max(fromMin, (min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return lerp(toMin, toMax, val);
    }

    //generate perlin noise texture
    void initializeTerrainTexture(float startScale, float heightScale)
    {
        //8MB array, does not fit on stack so heap allocate it
        float *pixels = new float[TERRAIN_TEXTURE_SIZE * TERRAIN_TEXTURE_SIZE];

        int nbLayers = 16;
        float maxValue = 0.0f;
        float amplitude = 1.0f;
        float scale = 0.0f;
        float noise = 0.0f;

        float x, z, y = 0.1f;

        for(int xCount = 0; xCount < TERRAIN_TEXTURE_SIZE; ++xCount)
        {
            x = float(xCount) / float(TERRAIN_TEXTURE_SIZE);
            for(int zCount = 0; zCount < TERRAIN_TEXTURE_SIZE; ++zCount)
            {
                z = float(zCount) / float(TERRAIN_TEXTURE_SIZE);
                noise = 0.0f;
                amplitude = 1.0f;
                scale = startScale;
                maxValue = 0.0f;

                for(int l = 0; l < nbLayers; ++l)
                {
                    float tmpNoise = stb_perlin_noise3(x * scale, y * scale, z * scale) * amplitude;
                    noise += tmpNoise;
                    maxValue += amplitude;
                    amplitude *= 0.5f;
                    scale *= 2.0f;
                }

                float res = noise / maxValue;
                res  = mapToRange(-0.4f, 0.8f, 0.0f, 1.0f, res);
                pixels[xCount * TERRAIN_TEXTURE_SIZE + zCount] = res;// * heightScale;
            }
        }

        glGenTextures(1, &(terrainGLData.terrainTexture));
        glBindTexture(GL_TEXTURE_2D, terrainGLData.terrainTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, TERRAIN_TEXTURE_SIZE, TERRAIN_TEXTURE_SIZE, 0,
                     GL_RED, GL_FLOAT, pixels);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

        delete[] pixels;
    }

    void initializeRenderData()
    {
        terrainGLData.terrainProgram = SCEShaders::CreateShaderProgram(TERRAIN_SHADER_NAME);

        GLuint terrainProgram = terrainGLData.terrainProgram;

        terrainGLData.terrainTextureUniform = glGetUniformLocation(terrainProgram, TERRAIN_TEXTURE_NAME);
        terrainGLData.viewMatrixUniform = glGetUniformLocation(terrainProgram, TERRAIN_VIEW_UNIFORM_NAME);
        terrainGLData.projectionMatrixUniform = glGetUniformLocation(terrainProgram, TERRAIN_PROJECTION_UNIFORM_NAME);

        terrainQuadUniforms.quadToTerrainMatrix = glGetUniformLocation(terrainProgram, QUAD_TO_TERRAIN_UNIFORM_NAME);
        terrainQuadUniforms.modelMatrix = glGetUniformLocation(terrainProgram, TERRAIN_MODEL_UNIFORM_NAME);
        terrainQuadUniforms.MVPMatrix = glGetUniformLocation(terrainProgram, TERRAIN_MVP_UNIFORM_NAME);

        //vertices buffer
        glGenBuffers(1, &quadVerticesVbo);
        glBindBuffer(GL_ARRAY_BUFFER, quadVerticesVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //indices buffer
        glGenBuffers(1, &quadIndicesVbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndicesVbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadPatchIndices), quadPatchIndices, GL_STATIC_DRAW);
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

        // We work with 4 points per patch.
        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }

    void renderPatch(const glm::mat4& projectionMatrix,
                     const glm::mat4& viewMatrix,
                     const glm::mat4& terrainToWorldspace,
                     const glm::vec3& position_terrainSpace,
                     float patchSize,
                     float halfTerrainSize)
    {
        //create matrix to convert vertex pos to terrain space coord
        glm::mat4 quadToTerrainspace = glm::scale(mat4(1.0f), glm::vec3(1.0f / halfTerrainSize)) *
                                       glm::translate(mat4(1.0f), position_terrainSpace) *
                                       glm::scale(mat4(1.0f), glm::vec3(patchSize));

        //Model to world matrix is concatenation of quad an terrain matrices
        glm::mat4 modelMatrix = terrainToWorldspace *
                                glm::translate(mat4(1.0f), position_terrainSpace) *
                                glm::scale(mat4(1.0f), glm::vec3(patchSize));
        glm::mat4 MVPMat = projectionMatrix * viewMatrix * modelMatrix;

        //do the rendering
        glUniformMatrix4fv(terrainQuadUniforms.quadToTerrainMatrix, 1, GL_FALSE, &(quadToTerrainspace[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.modelMatrix,         1, GL_FALSE, &(modelMatrix[0][0]));
        glUniformMatrix4fv(terrainQuadUniforms.MVPMatrix,           1, GL_FALSE, &(MVPMat[0][0]));

        glDrawElements(GL_PATCHES,
                       4,//indices count
                       GL_UNSIGNED_SHORT,
                       0);
    }

    void RenderTerrain(const glm::vec3& cameraPosition,
                       const glm::mat4& projectionMatrix,
                       const glm::mat4& viewMatrix,
                       float terrainBaseHeight)
    {
        float terrainSize = 500.0f;
        float halfTerrainSize = terrainSize / 2.0f;
        float patchSize = 20.0f;

        //setup gl state that at common for all patches
        glUseProgram(terrainGLData.terrainProgram);
        SCEShaders::BindDefaultUniforms(terrainGLData.terrainProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainGLData.terrainTexture);

        glUniform1i(terrainGLData.terrainTextureUniform, 0);//terrain height map is sampler 0
        glUniformMatrix4fv(terrainGLData.viewMatrixUniform,        1, GL_FALSE, &(viewMatrix[0][0]));
        glUniformMatrix4fv(terrainGLData.projectionMatrixUniform,  1, GL_FALSE, &(projectionMatrix[0][0]));

        glBindVertexArray(quadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndicesVbo);

        //we want the terrain mesh to "follow" the camera (the height map coordinates will be offset"
        glm::vec3 terrainPosition_worldspace = vec3(0.0f);//cameraPosition;
        terrainPosition_worldspace.y = terrainBaseHeight;

        glm::mat4 terrainToWorldspace = glm::translate(glm::mat4(1.0f), terrainPosition_worldspace);

        //Render all the terrain patches
        for(float x = -halfTerrainSize; x < halfTerrainSize; x += patchSize)
        {
            for(float z = -halfTerrainSize; z < halfTerrainSize; z += patchSize)
            {
                glm::vec3 pos_terrainspace(x,
                                           0.0f,
                                           z);
                renderPatch(projectionMatrix,
                            viewMatrix,
                            terrainToWorldspace,
                            pos_terrainspace,
                            patchSize,
                            halfTerrainSize);
            }
        }

        glBindVertexArray(0);
    }

    void Init()
    {
        initializeRenderData();
        initializeTerrainTexture(10.0f, 20.0f);
    }

    void Cleanup()
    {
        cleanup();
    }
}

}
