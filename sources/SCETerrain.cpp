/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.cpp**********/
/**************************************/


#include "../headers/SCETerrain.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCERenderStructs.hpp"
#include "../headers/SCETools.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>


#define TERRAIN_SHADER_NAME "TerrainTess"
#define TERRAIN_TEXTURE_NAME "TerrainHeightMap"
#define TERRAIN_MAX_DIST_UNIFORM_NAME "TerrainMaxDistance"
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
        GLint terrainMaxDistanceUniform;
        GLint viewMatrixUniform;
        GLint projectionMatrixUniform;
    };

    struct TerrainData
    {
        TerrainData()
            : quadVertices {
                  glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(1.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f)},
              quadPatchIndices{0, 1, 2, 3}
        {}

        //Terrain quad render data
        GLuint  quadVao;
        GLuint  quadVerticesVbo;
        GLuint  quadIndicesVbo;

        TerrainGLData glData;
        TerrainQuadUniforms quadUniforms;
        float terrainSize;
        float patchSize;
        float baseHeight;

        glm::vec3 quadVertices[4];
        ushort quadPatchIndices[4]; //indices for a quad, not 2 triangles
    };

    static TerrainData* terrainData;

    void cleanupGLData()
    {
        if(terrainData->glData.terrainTexture != GLuint(-1))
        {
            glDeleteTextures(1, &(terrainData->glData.terrainTexture));
        }

        if(terrainData->glData.terrainProgram != GLuint(-1))
        {
            SCEShaders::DeleteShaderProgram(terrainData->glData.terrainProgram);
        }
    }

    void computeNormalsForQuad(int xCount, int zCount, glm::vec3 *normals, float* heightmap)
    {
        float terrainSize = 1.0;
        float x, z;
        float y1, y2, y3, y4;
        float stepSize = terrainSize / float(TERRAIN_TEXTURE_SIZE);

        //hack to force normals to correspond to in game face normals
        float yScale = terrainSize/40.0f; //20.0f;

        x = (float(xCount) / float(TERRAIN_TEXTURE_SIZE) - 0.5f) * terrainSize;
        z = (float(zCount) / float(TERRAIN_TEXTURE_SIZE) - 0.5f) * terrainSize;

        int xOff = xCount < TERRAIN_TEXTURE_SIZE - 1 ? 1 : 0;
        int zOff = zCount < TERRAIN_TEXTURE_SIZE - 1 ? 1 : 0;

        y1 = heightmap[xCount * TERRAIN_TEXTURE_SIZE + zCount] * yScale;
        y2 = heightmap[(xCount + xOff) * TERRAIN_TEXTURE_SIZE + zCount] * yScale;
        y3 = heightmap[xCount * TERRAIN_TEXTURE_SIZE + (zCount + zOff)] * yScale;
        y4 = heightmap[(xCount + xOff) * TERRAIN_TEXTURE_SIZE + (zCount + zOff)] * yScale;

        glm::vec3 p1(x           , y1, z);
        glm::vec3 p2(x + stepSize, y2, z);
        glm::vec3 p3(x           , y3, z + stepSize);
        glm::vec3 p4(x + stepSize, y4, z + stepSize);

        glm::vec3 normal1 = cross(p3 - p1, p2 - p1);
        glm::vec3 normal2 = cross(p2 - p4, p3 - p4);

        //first is lower left corner
        normals[(xCount*TERRAIN_TEXTURE_SIZE + zCount)*2] = normal1;
        //second is upper right corner
        normals[(xCount*TERRAIN_TEXTURE_SIZE + zCount)*2 + 1] = normal2;

        /* following this pattern (le badass ascii art)
         *      |\--|1
         *      | \ |
         *     0|__\|
         */
    }

    //generate perlin noise texture
    void initializeTerrainTextures(float startScale, float heightScale)
    {
        //8MB array, does not fit on stack so heap allocate it
        float *heightmap = new float[TERRAIN_TEXTURE_SIZE * TERRAIN_TEXTURE_SIZE];
        glm::vec3 *normals = new glm::vec3[TERRAIN_TEXTURE_SIZE * TERRAIN_TEXTURE_SIZE * 2];
        glm::vec4 *packedHeightAndNormal = new glm::vec4[TERRAIN_TEXTURE_SIZE * TERRAIN_TEXTURE_SIZE];

        int nbLayers = 16;
        float maxValue = 0.0f;
        float amplitude = 1.0f;
        float scale = 0.0f;
        float noise = 0.0f;
        float decay = 0.4f;

        float x, z, y = 0.1f;
        float min = 10000.0f;
        float max = -10000.0f;

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
                    //stb_perlin returns values between -0.6 & 0.6
                    float tmpNoise = stb_perlin_noise3(x * scale, y * scale, z * scale) * amplitude;
                    noise += tmpNoise;
                    maxValue += amplitude;
                    amplitude *= decay;
                    scale *= 2.0f;
                }

                float res = noise / maxValue;
                res  = SCE::Math::mapToRange(-0.5f, 0.5f, 0.0f, 1.0f, res);
                min = res < min ? res : min;
                max = res > max ? res : max;
                heightmap[xCount * TERRAIN_TEXTURE_SIZE + zCount] = res * heightScale;
            }
        }

        //compute per face normal
        for(int xCount = 0; xCount < TERRAIN_TEXTURE_SIZE ; ++xCount)
        {
            for(int zCount = 0; zCount < TERRAIN_TEXTURE_SIZE; ++zCount)
            {
                computeNormalsForQuad(xCount, zCount, normals, heightmap);
            }
        }

        int lookX, lookZ;
        //compute smooth normals per vertex
        for(int xCount = 0; xCount < TERRAIN_TEXTURE_SIZE ; ++xCount)
        {
            for(int zCount = 0; zCount < TERRAIN_TEXTURE_SIZE; ++zCount)
            {
                glm::vec3 normalSum(0.0f);//normals[xCount * TERRAIN_TEXTURE_SIZE + zCount];

                //lower left tri
                if(xCount > 0 && zCount >0)
                {
                    lookX = xCount - 1;
                    lookZ = zCount - 1;
                    normalSum += normals[(lookX * TERRAIN_TEXTURE_SIZE + lookZ) * 2 + 1];
                }

                //upper right tri
                normalSum += normals[(xCount * TERRAIN_TEXTURE_SIZE + zCount) * 2];


                //lower right tri
                if(zCount >0)
                {
                    lookX = xCount;
                    lookZ = zCount - 1;
                    normalSum += normals[(lookX * TERRAIN_TEXTURE_SIZE + lookZ) * 2];
                    normalSum += normals[(lookX * TERRAIN_TEXTURE_SIZE + lookZ) * 2 + 1];
                }

                //upper left tri
                if(xCount > 0)
                {
                    lookX = xCount - 1;
                    lookZ = zCount;
                    normalSum += normals[(lookX * TERRAIN_TEXTURE_SIZE + lookZ) * 2];
                    normalSum += normals[(lookX * TERRAIN_TEXTURE_SIZE + lookZ) * 2 + 1];
                }

                y = heightmap[xCount * TERRAIN_TEXTURE_SIZE + zCount];
                packedHeightAndNormal[xCount * TERRAIN_TEXTURE_SIZE + zCount] =
                        glm::vec4(normalize(normalSum), y);
            }
        }

        glGenTextures(1, &(terrainData->glData.terrainTexture));
        glBindTexture(GL_TEXTURE_2D, terrainData->glData.terrainTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TERRAIN_TEXTURE_SIZE, TERRAIN_TEXTURE_SIZE, 0,
                     GL_RGBA, GL_FLOAT, packedHeightAndNormal);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);

        delete[] heightmap;
        delete[] normals;
        delete[] packedHeightAndNormal;
    }

    void initializeRenderData()
    {
        TerrainGLData& glData = terrainData->glData;
        TerrainQuadUniforms &quadUniforms = terrainData->quadUniforms;

        glData.terrainProgram = SCEShaders::CreateShaderProgram(TERRAIN_SHADER_NAME);

        GLuint terrainProgram = glData.terrainProgram;

        glData.terrainTextureUniform = glGetUniformLocation(terrainProgram, TERRAIN_TEXTURE_NAME);
        glData.viewMatrixUniform = glGetUniformLocation(terrainProgram, TERRAIN_VIEW_UNIFORM_NAME);
        glData.projectionMatrixUniform = glGetUniformLocation(terrainProgram,
                                                                     TERRAIN_PROJECTION_UNIFORM_NAME);
        glData.terrainMaxDistanceUniform = glGetUniformLocation(terrainProgram,
                                                                       TERRAIN_MAX_DIST_UNIFORM_NAME);

        quadUniforms.quadToTerrainMatrix = glGetUniformLocation(terrainProgram,
                                                                       QUAD_TO_TERRAIN_UNIFORM_NAME);
        quadUniforms.modelMatrix = glGetUniformLocation(terrainProgram, TERRAIN_MODEL_UNIFORM_NAME);
        quadUniforms.MVPMatrix = glGetUniformLocation(terrainProgram, TERRAIN_MVP_UNIFORM_NAME);

        //vertices buffer
        glGenBuffers(1, &(terrainData->quadVerticesVbo));
        glBindBuffer(GL_ARRAY_BUFFER, terrainData->quadVerticesVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(terrainData->quadVertices),
                     terrainData->quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //indices buffer
        glGenBuffers(1, &(terrainData->quadIndicesVbo));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->quadIndicesVbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainData->quadPatchIndices),
                     terrainData->quadPatchIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        //quad VAO creation
        glGenVertexArrays(1, &(terrainData->quadVao));
        glBindVertexArray(terrainData->quadVao);

        //setup VAO operations for automagic reuse
        glBindBuffer(GL_ARRAY_BUFFER, terrainData->quadVerticesVbo);
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
        TerrainQuadUniforms &quadUniforms = terrainData->quadUniforms;
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
        glUniformMatrix4fv(quadUniforms.quadToTerrainMatrix,
                           1, GL_FALSE, &(quadToTerrainspace[0][0]));
        glUniformMatrix4fv(quadUniforms.modelMatrix,
                           1, GL_FALSE, &(modelMatrix[0][0]));
        glUniformMatrix4fv(quadUniforms.MVPMatrix,
                           1, GL_FALSE, &(MVPMat[0][0]));

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

        TerrainGLData& glData = terrainData->glData;

        float halfTerrainSize = terrainData->terrainSize / 2.0f;
        float patchSize = terrainData->patchSize;

        //setup gl state that at common for all patches
        glUseProgram(glData.terrainProgram);
        SCEShaders::BindDefaultUniforms(glData.terrainProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glData.terrainTexture);

        glUniform1i(glData.terrainTextureUniform, 0);//terrain height map is sampler 0
        glUniform1f(glData.terrainMaxDistanceUniform, terrainData->terrainSize);
        glUniformMatrix4fv(glData.viewMatrixUniform,        1, GL_FALSE, &(viewMatrix[0][0]));
        glUniformMatrix4fv(glData.projectionMatrixUniform,  1, GL_FALSE, &(projectionMatrix[0][0]));

        glBindVertexArray(terrainData->quadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->quadIndicesVbo);

        //we want the terrain mesh to "follow" the camera (the height map coordinates will be offset ?)"
        glm::vec3 terrainPosition_worldspace = vec3(0.0f);//cameraPosition;
        terrainPosition_worldspace.y = terrainBaseHeight;

        glm::mat4 terrainToWorldspace = glm::translate(glm::mat4(1.0f), terrainPosition_worldspace);

        //Render all the terrain patches
        for(float x = -halfTerrainSize; x < halfTerrainSize - patchSize; x += patchSize)
        {
            for(float z = -halfTerrainSize; z < halfTerrainSize - patchSize; z += patchSize)
            {
                glm::vec3 pos_terrainspace(x, 0.0f, z);
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

    void Init(float terrainSize, float patchSize, float terrainBaseHeight)
    {
        terrainData = new TerrainData();
        terrainData->terrainSize = terrainSize;
        terrainData->patchSize = patchSize;
        terrainData->baseHeight = terrainBaseHeight;
        initializeRenderData();
        initializeTerrainTextures(3.0f, 5.0f);
    }

    void Cleanup()
    {
        cleanupGLData();
        delete terrainData;
    }
}

}
