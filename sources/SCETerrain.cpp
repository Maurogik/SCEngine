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
#include "../headers/SCEDebugText.hpp"
#include "../headers/SCE_GBuffer.hpp"
#include "../headers/SCEFrustrumCulling.hpp"
#include "../headers/SCETerrainShadow.hpp"
#include "../headers/SCETerrainTrees.hpp"
#include "../headers/SCEQuality.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#define USE_STB_PERLIN 1
#if USE_STB_PERLIN
#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>
#else
#include "../headers/SCEPerlin.hpp"
#endif


#define GRASS_TEX_FILE "Terrain/Textures/terrainGrass"
#define DIRT_TEX_FILE "Terrain/Textures/terrainDirt"
#define SNOW_TEX_FILE "Terrain/Textures/terrainSnow"

#define TERRAIN_SHADER_NAME "Terrain/TerrainTess"
#define TERRAIN_TEXTURE_UNIFORM "TerrainHeightMap"
#define GRASS_TEXTURE_UNIFORM "GrassTex"
#define DIRT_TEXTURE_UNIFORM "DirtTex"
#define SNOW_TEXTURE_UNIFORM "SnowTex"
#define TEX_TILE_SIZE_UNIFORM "TextureTileScale"
#define MAX_TESS_DIST_UNIFORM "MaxTessDistance"
#define WORLD_TO_TERRAIN_UNIFORM "WorldToTerrainSpace"
#define HEIGHT_SCALE_UNIFORM "HeightScale"
#define TESS_LOD_MULT_UNIFORM "TessLodMultiplier"
#define PATCH_SIZE_UNIFORM "PatchSize"

#ifdef SCE_DEBUG
#define TERRAIN_TEXTURE_SIZE 512
#else
#define TERRAIN_TEXTURE_SIZE 2048
#endif

//#define TERRAIN_TEXTURE_SIZE 2048
//#define TERRAIN_TEXTURE_SIZE 512
#define TEX_TILE_SIZE 2.0f

#define DISPLAY_TREES 1

//#define ISLAND_MODE

namespace SCE
{

namespace Terrain
{


/*  Translation unit local functions, with hidden names to avoid name conflicts in case of unity build */
    namespace
    {

        struct TerrainGLData
        {
            GLuint  terrainProgram;

            GLuint  terrainTexture;
            GLuint  grassTexture;
            GLuint  dirtTexture;
            GLuint  snowTexture;

            GLint   terrainTextureUniform;
            GLint   maxTesselationDistanceUniform;
            GLint   heightScaleUniform;
            GLint   tessLodMultUniform;
            GLint   grassTextureUniform;
            GLint   dirtTextureUniform;
            GLint   snowTextureUniform;
            GLint   textureTileScaleUniform;
            GLint   worldToTerrainMatUniform;
            GLint   patchSizeUniform;
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
                quadVertices[1] = glm::vec3(1.0f, 0.0f, 0.0f);
                quadVertices[2] = glm::vec3(1.0f, 0.0f, 1.0f);
                quadVertices[3] = glm::vec3(0.0f, 0.0f, 1.0f);

                heightmap = nullptr;
            }

            //Terrain quad render data
            GLuint  quadVao;
            GLuint  quadVerticesVbo;
            GLuint  quadIndicesVbo;

            float terrainSize;
            float maxTesselationDist;
            float patchSize;
            float baseHeight;
            float heightScale;
            glm::mat4 worldToTerrainspace;

            ushort quadPatchIndices[4]; //indices for a quad, not 2 triangles
            glm::vec3 quadVertices[4];

            TerrainGLData   glData;

            float *heightmap;
            std::vector<glm::mat4> patchModelMatrices;
            std::vector<glm::vec4> patchBoundingBoxCenters;

            TerrainShadow terrainShadow;
            TerrainTrees terrainTrees;
        };

 /*      File scope variables    */
        static TerrainData* terrainData = nullptr;

        void cleanupGLData()
        {
            if(terrainData->glData.terrainTexture != GL_INVALID_INDEX)
            {
                glDeleteTextures(1, &(terrainData->glData.terrainTexture));
            }

            glDeleteBuffers(1, &(terrainData->quadIndicesVbo));
            glDeleteBuffers(1, &(terrainData->quadVerticesVbo));
            glDeleteVertexArrays(1, &(terrainData->quadVao));

            if(terrainData->glData.terrainProgram != GL_INVALID_INDEX)
            {
                SCE::ShaderUtils::DeleteShaderProgram(terrainData->glData.terrainProgram);
            }

            if(terrainData->glData.grassTexture != GL_INVALID_INDEX)
            {
                SCE::TextureUtils::DeleteTexture(terrainData->glData.grassTexture);
            }

            if(terrainData->glData.dirtTexture != GL_INVALID_INDEX)
            {
                SCE::TextureUtils::DeleteTexture(terrainData->glData.dirtTexture);
            }

            if(terrainData->glData.snowTexture != GL_INVALID_INDEX)
            {
                SCE::TextureUtils::DeleteTexture(terrainData->glData.snowTexture);
            }
        }

        void computeNormalsForQuad(int xCount, int zCount, glm::vec3 *normals, float* heightmap)
        {
            float terrainSize = terrainData->terrainSize;
            float x, z;
            float y1, y2, y3, y4;
            float stepSize = terrainSize / float(TERRAIN_TEXTURE_SIZE);

            x = float(xCount)*stepSize;
            z = float(zCount)*stepSize;

            int xOff = xCount < TERRAIN_TEXTURE_SIZE - 1 ? 1 : 0;
            int zOff = zCount < TERRAIN_TEXTURE_SIZE - 1 ? 1 : 0;

            y1 = heightmap[xCount*TERRAIN_TEXTURE_SIZE + zCount];
            y2 = heightmap[(xCount + xOff)*TERRAIN_TEXTURE_SIZE + zCount];
            y3 = heightmap[xCount*TERRAIN_TEXTURE_SIZE + (zCount + zOff)];
            y4 = heightmap[(xCount + xOff)*TERRAIN_TEXTURE_SIZE + (zCount + zOff)];

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
            *     |\--|1
            *     | \ |
            *    0|__\|
            */
        }


        //generate perlin noise texture
        void initializeTerrain(float xOffset, float zOffset, float startScale, float heightScale)
        {
            //8MB array, does not fit on stack so heap allocate it
            float *heightmap = new float[TERRAIN_TEXTURE_SIZE*TERRAIN_TEXTURE_SIZE];
            glm::vec3 *normals = new glm::vec3[TERRAIN_TEXTURE_SIZE*TERRAIN_TEXTURE_SIZE*2];
            glm::vec4 *packedNormalAndHeight = new glm::vec4[TERRAIN_TEXTURE_SIZE*TERRAIN_TEXTURE_SIZE];

            int nbLayers = 16;
            float maxValue = 0.0f;
            float amplitude = 1.0f;
            float scale = 0.0f;
            float noise = 0.0f;
            float persistence = 0.3f;

            float x, z;
            float y = 0.0f;

            float edgeChange = 1.0f;
#ifdef ISLAND_MODE
            float xDist, zDist;
#endif            

            //Fill terrain heightmap
            for(int xCount = 0; xCount < TERRAIN_TEXTURE_SIZE; ++xCount)
            {
                x = float(xCount) / float(TERRAIN_TEXTURE_SIZE);
                x += xOffset;

                for(int zCount = 0; zCount < TERRAIN_TEXTURE_SIZE; ++zCount)
                {
                     z = float(zCount) / float(TERRAIN_TEXTURE_SIZE);
                     z += zOffset;
#ifdef ISLAND_MODE
                    xDist = (0.5f - x);
                    zDist = (0.5f - z);
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
#if USE_STB_PERLIN
                        float tmpNoise = stb_perlin_noise3(x*scale, y*scale, z*scale);
                        tmpNoise = SCE::Math::MapToRange(-0.7f, 0.7f, 0.0f, 1.0f, tmpNoise);
#else
                        float tmpNoise = Perlin::GetPerlinAt(x*scale, z*scale);
                        tmpNoise = SCE::Math::MapToRange(-0.5f, 0.5f, 0.0f, 1.0f, tmpNoise);
#endif
                        if(l == 0)
                        {
                            tmpNoise = glm::pow(tmpNoise, 4.0f);
                        }
                        else if(l == 1)
                        {
                            tmpNoise = glm::pow(tmpNoise, 2.0f);
                        }
//                        tmpNoise = glm::pow(tmpNoise, 1.0f + 2.0f*(1.0f - float(l)/float(nbLayers)));
                        noise += tmpNoise*amplitude;
                        maxValue += amplitude;
                        amplitude *= persistence*(0.75f + tmpNoise);
                        scale *= 2.0f;
                    }

                    float res = noise / maxValue;
#ifdef ISLAND_MODE
                    res = SCE::Math::mapToRange(0.4f, 1.0f, 0.0f, 1.0f, res);
#else
                    res = SCE::Math::MapToRange(0.0f, 1.0f, 0.0f, 1.0f, res);
#endif
                    heightmap[xCount*TERRAIN_TEXTURE_SIZE + zCount] = res*heightScale*edgeChange;
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
                    glm::vec3 normalSum(0.0f);//normals[xCount*TERRAIN_TEXTURE_SIZE + zCount];

                    //lower left tri
                    if(xCount > 0 && zCount >0)
                    {
                        lookX = xCount - 1;
                        lookZ = zCount - 1;
                        normalSum += normals[(lookX*TERRAIN_TEXTURE_SIZE + lookZ)*2 + 1];
                    }
                    //upper right tri
                    normalSum += normals[(xCount*TERRAIN_TEXTURE_SIZE + zCount)*2];
                    //lower right tri
                    if(zCount >0)
                    {
                        lookX = xCount;
                        lookZ = zCount - 1;
                        normalSum += normals[(lookX*TERRAIN_TEXTURE_SIZE + lookZ)*2];
                        normalSum += normals[(lookX*TERRAIN_TEXTURE_SIZE + lookZ)*2 + 1];
                    }
                    //upper left tri
                    if(xCount > 0)
                    {
                        lookX = xCount - 1;
                        lookZ = zCount;
                        normalSum += normals[(lookX*TERRAIN_TEXTURE_SIZE + lookZ)*2];
                        normalSum += normals[(lookX*TERRAIN_TEXTURE_SIZE + lookZ)*2 + 1];
                    }

                    y = heightmap[xCount*TERRAIN_TEXTURE_SIZE + zCount];
                    packedNormalAndHeight[xCount*TERRAIN_TEXTURE_SIZE + zCount] =
                            glm::vec4(normalize(normalSum), y);
                }
            }

            terrainData->terrainTrees.InitializeTreeLayout(packedNormalAndHeight, TERRAIN_TEXTURE_SIZE,
                                                           xOffset, zOffset,
                                                           startScale, heightScale,
                                                           terrainData->terrainSize*0.5f);

            glGenTextures(1, &(terrainData->glData.terrainTexture));
            glBindTexture(GL_TEXTURE_2D, terrainData->glData.terrainTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TERRAIN_TEXTURE_SIZE, TERRAIN_TEXTURE_SIZE, 0,
                         GL_RGBA, GL_FLOAT, packedNormalAndHeight);

            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);

            //delete textures, except for the heightmap which is used at runtime
            //and will be deleted in the cleanup function
            delete[] normals;
            delete[] packedNormalAndHeight;
            terrainData->heightmap = heightmap;
        }

        void initializeRenderData()
        {
            TerrainGLData& glData = terrainData->glData;

            GLuint terrainProgram = SCE::ShaderUtils::CreateShaderProgram(TERRAIN_SHADER_NAME);

             glData.terrainProgram = terrainProgram;

            glData.terrainTextureUniform = glGetUniformLocation(terrainProgram, TERRAIN_TEXTURE_UNIFORM);
            glData.grassTextureUniform = glGetUniformLocation(terrainProgram, GRASS_TEXTURE_UNIFORM);
            glData.dirtTextureUniform = glGetUniformLocation(terrainProgram, DIRT_TEXTURE_UNIFORM);
            glData.snowTextureUniform = glGetUniformLocation(terrainProgram, SNOW_TEXTURE_UNIFORM);
            glData.textureTileScaleUniform = glGetUniformLocation(terrainProgram, TEX_TILE_SIZE_UNIFORM);
            glData.maxTesselationDistanceUniform =
                    glGetUniformLocation(terrainProgram, MAX_TESS_DIST_UNIFORM);
            glData.heightScaleUniform = glGetUniformLocation(terrainProgram, HEIGHT_SCALE_UNIFORM);
            glData.tessLodMultUniform =
                    glGetUniformLocation(terrainProgram, TESS_LOD_MULT_UNIFORM);

            glData.worldToTerrainMatUniform =
                    glGetUniformLocation(terrainProgram, WORLD_TO_TERRAIN_UNIFORM);
            glData.patchSizeUniform =
                    glGetUniformLocation(terrainProgram, PATCH_SIZE_UNIFORM);

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
            GLuint vertexAttribLocation = glGetAttribLocation(terrainProgram,
                                                              "vertexPosition_modelspace");
            glEnableVertexAttribArray(vertexAttribLocation);
            glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glBindVertexArray(0);

            // We work with 4 points per patch.
            glPatchParameteri(GL_PATCH_VERTICES, 4);

        }

        void renderPatch(const glm::mat4& projectionMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& modelMatrix)
        {
            //do the rendering
            SCE::ShaderUtils::BindDefaultUniforms(terrainData->glData.terrainProgram, modelMatrix,
                                                  viewMatrix, projectionMatrix);
            glDrawElements(GL_PATCHES,
                           4,//indices count
                           GL_UNSIGNED_SHORT,
                           0);
        }

        //pre-compute model matrices for all patches
        void initializePatchMatrices()
        {
            float halfTerrainSize = terrainData->terrainSize*0.5f;
            float patchSize = terrainData->patchSize;

            terrainData->patchModelMatrices.reserve(int(terrainData->terrainSize/terrainData->patchSize));

            glm::vec3 terrainPosition_worldspace = vec3(0.0);
            terrainPosition_worldspace.y = terrainData->baseHeight;
            glm::mat4 terrainToWorldspace = glm::translate(glm::mat4(1.0f), terrainPosition_worldspace);

            for(float x = -halfTerrainSize + terrainData->patchSize*0.5f;
                x < halfTerrainSize - terrainData->patchSize;
                x += terrainData->patchSize)
            {
                for(float z = -halfTerrainSize + terrainData->patchSize*0.5f;
                    z < halfTerrainSize - terrainData->patchSize;
                    z += terrainData->patchSize)
                {
                    glm::vec3 pos_worldspace(x, 0.0f, z);

                    //Model to world matrix is concatenation of quad and terrain matrices
                    glm::mat4 modelMatrix = terrainToWorldspace *
                                            glm::translate(mat4(1.0f), pos_worldspace) *
                                            glm::scale(mat4(1.0f), glm::vec3(terrainData->patchSize));

                    pos_worldspace += glm::vec3(patchSize, patchSize, patchSize)*0.5f;
                    float height = GetTerrainHeight(pos_worldspace);
                    pos_worldspace.y = height;

                    terrainData->patchModelMatrices.push_back(modelMatrix);
                    terrainData->patchBoundingBoxCenters.push_back(glm::vec4(pos_worldspace, 1.0));
                }
            }
        }


    //end of anonymous namespace
    }

    void UpdateTerrain(const glm::mat4& projectionMatrix,
                       const glm::mat4& viewMatrix)
    {
        //TODO find a better way to not render when there is no terrain
        if(!terrainData)
        {
            return;
        }

        glm::vec3 cameraPosition = glm::vec3(glm::inverse(viewMatrix)*glm::vec4(0.0, 0.0, 0.0, 1.0));
        SCE::DebugText::LogMessage("Cam : " + std::to_string(cameraPosition.x)
                              + ", " + std::to_string(cameraPosition.y)
                              + ", " + std::to_string(cameraPosition.z));

#if DISPLAY_TREES

        float patchSize = terrainData->patchSize;
        float terrainSize = terrainData->terrainSize;
        //compute the actual terrain size we will cover with patches
        terrainSize = floor(terrainSize/patchSize)*patchSize;
        float halfTerrainSize = terrainSize / 2.0f;

        float maxDistFromCenter = halfTerrainSize - patchSize * 0.5;

        terrainData->terrainTrees.SpawnTreeInstances(viewMatrix, terrainData->worldToTerrainspace,
                                                     cameraPosition, maxDistFromCenter);
#endif
    }

    void RenderTerrain(const glm::mat4& projectionMatrix,
                       const glm::mat4& viewMatrix,
                       bool isShadowPass)
    {
        //TODO find a better way to not render when there is no terrain
        if(!terrainData)
        {
            return;
        }

        if(isShadowPass)
        {
            SCE::DebugText::SetDefaultPrintColor(glm::vec3(0.0, 0.0, 0.5));
        }
        else
        {
            SCE::DebugText::SetDefaultPrintColor(glm::vec3(0.0, 0.0, 0.0));
        }

        TerrainGLData& glData = terrainData->glData;        

        //setup gl state that is common for all patches
        glUseProgram(glData.terrainProgram);

        //bind terrain textures
        SCE::TextureUtils::BindSafeTexture(glData.terrainTexture, 0, 0);//terrain height map is sampler 0
        SCE::TextureUtils::BindTexture(glData.grassTexture, 1, glData.grassTextureUniform);
        SCE::TextureUtils::BindTexture(glData.dirtTexture, 2, glData.dirtTextureUniform);
        SCE::TextureUtils::BindTexture(glData.snowTexture, 3, glData.snowTextureUniform);

        //uniforms        
        glUniform1f(glData.maxTesselationDistanceUniform, terrainData->terrainSize);
        glUniform1f(glData.heightScaleUniform, terrainData->heightScale);
        glUniform1f(glData.tessLodMultUniform, isShadowPass ? 32.0f : SCE::Quality::TerrainLodMultiplier);
        glUniform1f(glData.textureTileScaleUniform, terrainData->terrainSize / TEX_TILE_SIZE);
        glUniform1f(glData.patchSizeUniform, terrainData->patchSize);

        glUniformMatrix4fv(glData.worldToTerrainMatUniform, 1, GL_FALSE,
                           &(terrainData->worldToTerrainspace[0][0]));

        glBindVertexArray(terrainData->quadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->quadIndicesVbo);

        float patchSize = terrainData->patchSize;

        glm::vec4 R_cameraspace = viewMatrix*glm::vec4(patchSize, 0.0, 0.0, 0.0);
        glm::vec4 S_cameraspace = viewMatrix*glm::vec4(0.0, patchSize, 0.0, 0.0);
        glm::vec4 T_cameraspace = viewMatrix*glm::vec4(0.0, 0.0, patchSize, 0.0);

        uint patchCount = 0;
        uint offscreenCount = 0;
        for(uint i = 0; i < terrainData->patchModelMatrices.size(); ++i)
        {
            glm::vec4 center_cameraspace = viewMatrix*terrainData->patchBoundingBoxCenters[i];

            if(SCE::FrustrumCulling::IsBoxInFrustrum(center_cameraspace,
                                                     R_cameraspace, S_cameraspace, T_cameraspace))
            {
                renderPatch(projectionMatrix,
                            viewMatrix,
                            terrainData->patchModelMatrices[i]);
            }
            else
            {
                ++offscreenCount;
            }
            ++patchCount;
        }

        SCE::DebugText::LogMessage("Rendering terrain");
        SCE::DebugText::LogMessage("Patches rendered : " + std::to_string(patchCount));
        SCE::DebugText::LogMessage("Patches offscreen : " + std::to_string(offscreenCount));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void RenderTrees(const glm::mat4& projectionMatrix,
                     const glm::mat4& viewMatrix,
                     bool isShadowPass)
    {
        //TODO find a better way to not render when there is no terrain
        if(!terrainData)
        {
            return;
        }

#if DISPLAY_TREES
        terrainData->terrainTrees.RenderTrees(projectionMatrix, viewMatrix, isShadowPass);
#endif

    }

    void RenderShadow(const mat4 &projectionMatrix, const mat4 &viewMatrix,
                      const glm::vec3 &sunPosition, SCE_GBuffer &gbuffer)
    {
        //TODO find a better way to not render when there is no terrain
        if(!terrainData)
        {
            return;
        }

        //only a screen space quad, don't need depth testing
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);

        terrainData->terrainShadow.RenderShadow(projectionMatrix, viewMatrix,
                                                sunPosition, gbuffer, terrainData->worldToTerrainspace,
                                                terrainData->glData.terrainTexture,
                                                terrainData->heightScale);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }

    void Init(float terrainSize, float patchSize, float terrainBaseHeight, float maxTessDist)
    {
        if(!terrainData)
        {
#if !USE_STB_PERLIN
            Perlin::MakePerlin(TERRAIN_TEXTURE_SIZE, TERRAIN_TEXTURE_SIZE);
#endif

            //compute the actual terrain size we will cover with patches
            terrainSize = floor(terrainSize/patchSize)*patchSize;
            float halfTerrainSize = terrainSize / 2.0f;

            terrainData = new TerrainData();
            terrainData->terrainSize = terrainSize;
            terrainData->patchSize = patchSize;
            terrainData->baseHeight = terrainBaseHeight;
            terrainData->heightScale = 1400.0f;
            terrainData->maxTesselationDist = maxTessDist;

            glm::vec3 terrainPosition_worldspace = vec3(0.0);
            terrainPosition_worldspace.y = terrainData->baseHeight;
            //create matrix to convert from woldspace to terrain space coord
            terrainData->worldToTerrainspace = glm::scale(mat4(1.0f), glm::vec3(1.0f / (halfTerrainSize)))
                    * glm::translate(mat4(1.0f), -terrainPosition_worldspace);

            initializeRenderData();
            float xOffset = SCE::Math::RandRange(0.0f, 1.0f);
            float zOffset = SCE::Math::RandRange(0.0f, 1.0f);
            initializeTerrain(xOffset, zOffset, 2.0f*terrainSize / 4000.0f,
                                      terrainData->heightScale);
            initializePatchMatrices();
        }
    }

    void Cleanup()
    {
        if(terrainData)
        {
#if !USE_STB_PERLIN
            Perlin::DestroyPerlin();
#endif
            cleanupGLData();
            if(terrainData->heightmap != nullptr)
            {
                delete[] terrainData->heightmap;
            }
            delete terrainData;
            terrainData = nullptr;
        }
    }

    float GetTerrainHeight(const vec3& pos_worldspace)
    {
        if(terrainData)
        {
            glm::vec4 pos_terrainSpace = terrainData->worldToTerrainspace*glm::vec4(pos_worldspace, 1.0);
            int x = int((pos_terrainSpace.x*0.5f + 0.5f)*TERRAIN_TEXTURE_SIZE);
            int z = int((pos_terrainSpace.z*0.5f + 0.5f)*TERRAIN_TEXTURE_SIZE);
            if(x >= 0 && x < TERRAIN_TEXTURE_SIZE && z >= 0 && z < TERRAIN_TEXTURE_SIZE)
            {
                return terrainData->heightmap[x*TERRAIN_TEXTURE_SIZE + z] + terrainData->baseHeight;
            }
        }
        return 0.0f;
    }


}

}
