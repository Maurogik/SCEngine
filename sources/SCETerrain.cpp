/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETerrain.cpp**********/
/**************************************/


#include "../headers/SCETerrain.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCETextures.hpp"
#include "../headers/SCERenderStructs.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCETime.hpp"
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCEMeshRender.hpp"

#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#define GRASS_TEX_FILE "Terrain/Textures/terrainGrass"
#define DIRT_TEX_FILE "Terrain/Textures/terrainDirt"
#define SNOW_TEX_FILE "Terrain/Textures/terrainSnow"

#define TERRAIN_SHADER_NAME "Terrain/TerrainTess"
#define TERRAIN_TEXTURE_UNIFORM "TerrainHeightMap"
#define GRASS_TEXTURE_UNIFORM "GrassTex"
#define DIRT_TEXTURE_UNIFORM "DirtTex"
#define SNOW_TEXTURE_UNIFORM "SnowTex"
#define TEX_TILE_SIZE_UNIFORM "TextureTileScale"
#define PATCH_SIZE_UNIFORM "PatchSize"
#define TERRAIN_MAX_DIST_UNIFORM "TerrainMaxDistance"
#define QUAD_TO_TERRAIN_UNIFORM "QuadToTerrainSpace"
#define TREE_TO_TERRAIN_UNIFORM "TreeToTerrainSpace"
#define HEIGHT_SCALE_UNIFORM "HeightScale"
#define TESS_OVERRIDE_UNIFORM "TesselationOverride"

#define TREE_SHADER_NAME "Terrain/Tree"
#define TREE_MODEL_NAME "Terrain/Meshes/low_poly_tree.obj"

#define TERRAIN_TEXTURE_SIZE 4096
//#define TERRAIN_TEXTURE_SIZE 2048
//#define TERRAIN_TEXTURE_SIZE 512
#define TEX_TILE_SIZE 2.0f

//#define ISLAND_MODE

namespace SCE
{

namespace Terrain
{
    struct TerrainQuadUniforms
    {
        GLint quadToTerrainMatrix;
    };

    struct TerrainGLData
    {
        GLuint terrainProgram;
        GLuint terrainTexture;
        GLuint grassTexture;
        GLuint dirtTexture;
        GLuint snowTexture;
        GLint terrainTextureUniform;
        GLint terrainMaxDistanceUniform;
        GLint heightScaleUniform;
        GLint tesselationOverrideUniform;
        GLint grassTextureUniform;
        GLint dirtTextureUniform;
        GLint snowTextureUniform;
        GLint textureTileScaleUniform;
    };

    struct TreeGLData
    {
        ui16    treeMeshId;
        GLuint  treeShaderProgram;
        GLint   terrainTextureUniform;
        GLint   patchSizeUniform;
        GLint   treeToTerrainMatUniform;
    };

    struct TerrainData
    {
        TerrainData()
        {
			quadPatchIndices[0] = 0;
			quadPatchIndices[1] = 1;
			quadPatchIndices[2] = 2;
			quadPatchIndices[3] = 3;
			quadVertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);
			quadVertices[0] = glm::vec3(1.0f, 0.0f, 0.0f);
			quadVertices[0] = glm::vec3(1.0f, 0.0f, 1.0f);
			quadVertices[0] = glm::vec3(0.0f, 0.0f, 1.0f);
		}

        //Terrain quad render data
        GLuint  quadVao;
        GLuint  quadVerticesVbo;
        GLuint  quadIndicesVbo;

        float terrainSize;
        float patchSize;
        float baseHeight;
        float heightScale;

        ushort quadPatchIndices[4]; //indices for a quad, not 2 triangles
        glm::vec3 quadVertices[4];

        TerrainGLData   glData;
        TreeGLData      treeGlData;
        TerrainQuadUniforms quadUniforms;
    };

/*      File scope variables    */
    static TerrainData* terrainData = nullptr;

/*  Translation unit local functions, with hidden names to avoid name conflicts in case of unity build */
    namespace
    {
        void cleanupGLData()
        {
            if(terrainData->glData.terrainTexture != GLuint(-1))
            {
                glDeleteTextures(1, &(terrainData->glData.terrainTexture));
            }

            if(terrainData->glData.terrainProgram != GLuint(-1))
            {
                SCE::ShaderUtils::DeleteShaderProgram(terrainData->glData.terrainProgram);
            }

            if(terrainData->treeGlData.treeShaderProgram != GLuint(-1))
            {
                SCE::ShaderUtils::DeleteShaderProgram(terrainData->treeGlData.treeShaderProgram);
            }

//            SCE::MeshRender::DeleteMeshRenderData(terrainData->treeGlData.treeMeshId);
//            SCE::MeshLoader::DeleteMesh(terrainData->treeGlData.treeMeshId);
        }

        void computeNormalsForQuad(int xCount, int zCount, glm::vec3 *normals, float* heightmap)
        {
            float terrainSize = terrainData->terrainSize;
            float x, z;
            float y1, y2, y3, y4;
            float stepSize = terrainSize / float(TERRAIN_TEXTURE_SIZE);

            //hack to force normals to correspond to in game face normals
            float yScale = terrainData->patchSize;

            x = float(xCount) * stepSize;
            z = float(zCount) * stepSize;

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
        void initializeTerrainTextures(float xOffset, float zOffset, float startScale, float heightScale)
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
            float persistence = 0.3f;

            float x, z;
            float y = 0.0f;

            float edgeChange = 1.0f;
            float xDist, zDist;

            for(int xCount = 0; xCount < TERRAIN_TEXTURE_SIZE; ++xCount)
            {
                x = float(xCount) / float(TERRAIN_TEXTURE_SIZE);
                xDist = (0.5f - x);
                x += xOffset;

                for(int zCount = 0; zCount < TERRAIN_TEXTURE_SIZE; ++zCount)
                {
                     z = float(zCount) / float(TERRAIN_TEXTURE_SIZE);
                     zDist = (0.5f - z);
                     z += zOffset;
#ifdef ISLAND_MODE
                    float distToCenter = sqrt(xDist*xDist + zDist*zDist);//min(xDist, zDist);
                    edgeChange = SCE::Math::mapToRange(0.15f, 0.495f, 1.0f, 0.0f, distToCenter);
                    //ease in-out
                    edgeChange = 1.0f / (1.0f + exp(-(edgeChange - 0.5f)*8.0f));
#endif

                    noise = 0.0f;
                    amplitude = 1.0f;
                    scale = startScale;
                    maxValue = 0.0f;

                    for(int l = 0; l < nbLayers; ++l)
                    {
                        //stb_perlin returns values between -0.6 & 0.6
                        float tmpNoise = stb_perlin_noise3(x * scale, y * scale, z * scale);
                        tmpNoise = SCE::Math::mapToRange(-0.7f, 0.7f, 0.0f, 1.0f, tmpNoise);
                        noise += tmpNoise * amplitude;
                        maxValue += amplitude;
                        amplitude *= persistence * (0.75f + tmpNoise);
                        scale *= 2.0f;
                    }

                    float res = noise / maxValue;
#ifdef ISLAND_MODE
                    res = SCE::Math::mapToRange(0.4f, 1.0f, 0.0f, 1.0f, res);
#else
                    res = SCE::Math::mapToRange(0.0f, 1.0f, 0.0f, 1.0f, res);
#endif
                    heightmap[xCount * TERRAIN_TEXTURE_SIZE + zCount] = res * heightScale * edgeChange;
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

        bool isQuadOffscreen(const glm::mat4& MVPMat)
        {
            int onScreenCount = 4;
            float tolerance = terrainData->heightScale;
            for(int i = 0; i < 4; ++i)
            {
                glm::vec4 pos = MVPMat * glm::vec4(terrainData->quadVertices[i], 1.0);
                pos /= pos.w;
                //check if the patch is completely on screen
                // don't check the y pos because we will change it during tesselation
                if(pos.z > tolerance || pos.z < -tolerance ||
                   pos.x > tolerance || pos.x < -tolerance )
                {
                    --onScreenCount;
                }
            }

            return onScreenCount == 0;
        }

        void initializeRenderData()
        {
            TerrainGLData& glData = terrainData->glData;
            TerrainQuadUniforms &quadUniforms = terrainData->quadUniforms;

            GLuint terrainProgram = SCE::ShaderUtils::CreateShaderProgram(TERRAIN_SHADER_NAME);

             glData.terrainProgram = terrainProgram;

            glData.terrainTextureUniform = glGetUniformLocation(terrainProgram, TERRAIN_TEXTURE_UNIFORM);
            glData.grassTextureUniform = glGetUniformLocation(terrainProgram, GRASS_TEXTURE_UNIFORM);
            glData.dirtTextureUniform = glGetUniformLocation(terrainProgram, DIRT_TEXTURE_UNIFORM);
            glData.snowTextureUniform = glGetUniformLocation(terrainProgram, SNOW_TEXTURE_UNIFORM);
            glData.textureTileScaleUniform = glGetUniformLocation(terrainProgram, TEX_TILE_SIZE_UNIFORM);
            glData.terrainMaxDistanceUniform =
                    glGetUniformLocation(terrainProgram, TERRAIN_MAX_DIST_UNIFORM);
            glData.heightScaleUniform = glGetUniformLocation(terrainProgram, HEIGHT_SCALE_UNIFORM);
            glData.tesselationOverrideUniform =
                    glGetUniformLocation(terrainProgram, TESS_OVERRIDE_UNIFORM);

            quadUniforms.quadToTerrainMatrix =
                    glGetUniformLocation(terrainProgram, QUAD_TO_TERRAIN_UNIFORM);

            glData.grassTexture = SCE::TextureUtils::LoadTexture(GRASS_TEX_FILE);
            glData.dirtTexture = SCE::TextureUtils::LoadTexture(DIRT_TEX_FILE);
            glData.snowTexture = SCE::TextureUtils::LoadTexture(SNOW_TEX_FILE);

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

            //Load tree model
            ui16 meshId = SCE::MeshLoader::CreateMeshFromFile(TREE_MODEL_NAME);
            SCE::MeshRender::InitializeMeshRenderData(meshId);
            TreeGLData& treeData = terrainData->treeGlData;
            treeData.treeMeshId = meshId;
            treeData.treeShaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_SHADER_NAME);
            treeData.patchSizeUniform =
                    glGetUniformLocation(treeData.treeShaderProgram, PATCH_SIZE_UNIFORM);
            treeData.treeToTerrainMatUniform =
                    glGetUniformLocation(treeData.treeShaderProgram, TREE_TO_TERRAIN_UNIFORM);
            treeData.terrainTextureUniform =
                    glGetUniformLocation(treeData.treeShaderProgram, TERRAIN_TEXTURE_UNIFORM);


        }

        void renderPatch(const glm::mat4& projectionMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& terrainToWorldspace,
                         const glm::vec3& position_terrainSpace,
                         float patchSize,
                         float halfTerrainSize)
        {

            //create matrix to convert vertex pos to terrain space coord
            glm::mat4 quadToTerrainspace = glm::scale(mat4(1.0f), glm::vec3(1.0f / (halfTerrainSize))) *
                                           glm::translate(mat4(1.0f), position_terrainSpace) *
                                           glm::scale(mat4(1.0f), glm::vec3(patchSize));

            //Model to world matrix is concatenation of quad and terrain matrices
            glm::mat4 modelMatrix = terrainToWorldspace *
                                    glm::translate(mat4(1.0f), position_terrainSpace) *
                                    glm::scale(mat4(1.0f), glm::vec3(patchSize));
            glm::mat4 MVPMat = projectionMatrix * viewMatrix * modelMatrix;

            if(!isQuadOffscreen(MVPMat))
            {
                //do the rendering
                glUniformMatrix4fv(terrainData->quadUniforms.quadToTerrainMatrix,
                                   1, GL_FALSE, &(quadToTerrainspace[0][0]));
                SCE::ShaderUtils::BindDefaultUniforms(terrainData->glData.terrainProgram, modelMatrix,
                                                      viewMatrix, projectionMatrix);
                glDrawElements(GL_PATCHES,
                               4,//indices count
                               GL_UNSIGNED_SHORT,
                               0);
            }
        }


        void renderPatchTrees(const glm::mat4& projectionMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& terrainToWorldspace,
                         const glm::vec3& position_terrainSpace,
                         float patchSize,
                         float halfTerrainSize,
                         int patchCount)
        {
            glm::mat4 quadToTerrainspace = glm::scale(mat4(1.0f), glm::vec3(1.0f / (halfTerrainSize))) *
                                           glm::translate(mat4(1.0f), position_terrainSpace);

            glm::mat4 quadModelMatrix = terrainToWorldspace *
                                    glm::translate(mat4(1.0f), position_terrainSpace) *
                                    glm::scale(mat4(1.0f), glm::vec3(patchSize));
            glm::mat4 quadMVP = projectionMatrix * viewMatrix * quadModelMatrix;

            if(!isQuadOffscreen(quadMVP))
            {
                float originalSpacing = 50.0f;
                float spacing = originalSpacing;

                for(float x = 0.0f; x < patchSize; x += spacing)
                {
                    for(float z = 0.0f; z < patchSize; z += spacing)
                    {
                        glm::vec3 treePos(x, 0.0f, z);
                        mat4 treeToTerrainspace = quadToTerrainspace *
                                glm::translate(mat4(1.0f), treePos);

                        float spacingModif = 0.0f;

                        if(spacingModif > 0.00f)
                        {
//                          spacing = originalSpacing * spacingModif;
                            glUniformMatrix4fv(terrainData->treeGlData.treeToTerrainMatUniform,
                                               1, GL_FALSE, &(treeToTerrainspace[0][0]));

                            //render a tree
                            glm::mat4 modelMatrix = terrainToWorldspace *
                                                    glm::translate(mat4(1.0f), position_terrainSpace) *
                                                    glm::translate(mat4(1.0f), treePos);

                            SCE::MeshRender::RenderMesh(terrainData->treeGlData.treeMeshId,
                                                        projectionMatrix,
                                                        viewMatrix,
                                                        modelMatrix);
                        }
                    }
                }
            }
        }

    //end of anonymous namespace
    }


    void RenderTerrain(const glm::mat4& projectionMatrix,
                       const glm::mat4& viewMatrix,
                       float tesselationOverride)
    {
        if(!terrainData)
        {
            return;
        }

        TerrainGLData& glData = terrainData->glData;        

        float patchSize = terrainData->patchSize;
        float terrainSize = terrainData->terrainSize;
        //compute the actual terrain size we will cover with patches
        terrainSize = floor(terrainSize/patchSize) * patchSize;
        float halfTerrainSize = terrainSize / 2.0f;

        //setup gl state that at common for all patches
        glUseProgram(glData.terrainProgram);        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glData.terrainTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, glData.grassTexture);
        glUniform1i(glData.grassTextureUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, glData.dirtTexture);
        glUniform1i(glData.dirtTextureUniform, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, glData.snowTexture);
        glUniform1i(glData.snowTextureUniform, 3);

        glUniform1i(glData.terrainTextureUniform, 0);//terrain height map is sampler 0
        glUniform1f(glData.terrainMaxDistanceUniform, terrainSize);
        glUniform1f(glData.heightScaleUniform, terrainData->heightScale);
        glUniform1f(glData.tesselationOverrideUniform, tesselationOverride);
        glUniform1f(glData.textureTileScaleUniform, terrainSize / TEX_TILE_SIZE);

        glBindVertexArray(terrainData->quadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->quadIndicesVbo);

        glm::vec3 terrainPosition_worldspace = vec3(0.0);//glm::vec3(cameraPosition);
        terrainPosition_worldspace.y = terrainData->baseHeight;

        glm::mat4 terrainToWorldspace = glm::translate(glm::mat4(1.0f), terrainPosition_worldspace);

        //Render all the terrain patches
        for(float x = -halfTerrainSize + patchSize * 0.5f; x < halfTerrainSize - patchSize;
            x += patchSize)
        {
            for(float z = -halfTerrainSize + patchSize * 0.5f; z < halfTerrainSize - patchSize;
                z += patchSize)
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

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        TreeGLData& treeData = terrainData->treeGlData;
        glUseProgram(treeData.treeShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glData.terrainTexture);
        glUniform1i(treeData.terrainTextureUniform, 0);
        glUniform1f(treeData.patchSizeUniform, patchSize);

//        int patchCount = (int)floor(terrainData->terrainSize/terrainData->patchSize);
//        for(int x = 1; x < patchCount - 1; ++x)
//        {
//            for(int z = 1; z < patchCount - 1; ++z)
//            {
//                glm::vec3 pos_terrainspace((float)x * patchSize - halfTerrainSize, 0.0f,
//                                           (float)z * patchSize - halfTerrainSize);
//                renderPatchTrees(projectionMatrix,
//                                 viewMatrix,
//                                 terrainToWorldspace,
//                                 pos_terrainspace,
//                                 patchSize,
//                                 halfTerrainSize,
//                                 patchCount);
//            }
//        }
    }

    void Init(float terrainSize, float patchSize, float terrainBaseHeight)
    {
        if(!terrainData)
        {
            terrainData = new TerrainData();
            terrainData->terrainSize = terrainSize;
            terrainData->patchSize = patchSize;
            terrainData->baseHeight = terrainBaseHeight;
            terrainData->heightScale = 1200.0f / patchSize;
            initializeRenderData();
            float xOffset = SCE::Math::randRange(0.0f, 1.0f);
            float zOffset = SCE::Math::randRange(0.0f, 1.0f);
            initializeTerrainTextures(xOffset, zOffset, 2.0f * terrainSize / 3000.0f,
                                      terrainData->heightScale);
        }
    }

    void Cleanup()
    {
        if(terrainData)
        {
            cleanupGLData();
            delete terrainData;
            terrainData = nullptr;
        }
    }
}

}
