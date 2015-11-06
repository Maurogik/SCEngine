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
#include "../headers/SCEDebugText.hpp"

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
#define TERRAIN_MAX_DIST_UNIFORM "TerrainMaxDistance"
#define WORLD_TO_TERRAIN_UNIFORM "WorldToTerrainSpace"
#define HEIGHT_SCALE_UNIFORM "HeightScale"
#define TESS_OVERRIDE_UNIFORM "TesselationOverride"

#define TREE_SHADER_NAME "Terrain/Tree"
#define TREE_MODEL_NAME "Terrain/Meshes/tree_lod"
#define TREE_LOD_COUNT 3

#define IMPOSTOR_SHADER_NAME "Terrain/TreeImpostor"
#define IMPOSTOR_TEXTURE_UNIFORM "ImpostorTex"
#define IMPOSTOR_NORMAL_UNIFORM "ImpostorNormalTex"
#define IMPOSTOR_SCALE_INVERT_UNIFORM "ScaleInvertMat"
#define IMPOSTOR_TEXTURE_NAME "Terrain/Textures/treeImpostor"
#define IMPOSTOR_NORMAL_NAME "Terrain/Textures/treeImpostorNormal"
#define IMPOSTOR_SIZE 12.0f

//#define TERRAIN_TEXTURE_SIZE 4096
//#define TERRAIN_TEXTURE_SIZE 2048
#define TERRAIN_TEXTURE_SIZE 512
#define TEX_TILE_SIZE 2.0f

#define DISPLAY_TREES

//#define ISLAND_MODE

namespace SCE
{

namespace Terrain
{
    struct TerrainGLData
    {
        GLuint  terrainProgram;

        GLuint  terrainTexture;
        GLuint  grassTexture;
        GLuint  dirtTexture;
        GLuint  snowTexture;

        GLint   terrainTextureUniform;
        GLint   terrainMaxDistanceUniform;
        GLint   heightScaleUniform;
        GLint   tesselationOverrideUniform;
        GLint   grassTextureUniform;
        GLint   dirtTextureUniform;
        GLint   snowTextureUniform;
        GLint   textureTileScaleUniform;
        GLint   worldToTerrainMatUniform;
    };

    struct ImpostorGLData
    {
        ImpostorGLData()
            : scaleMatrix(glm::scale(mat4(1.0), glm::vec3(0.5, 1.0, 1.0)))
        {}

        ui16        meshId;
        GLuint      texture;
        GLint       textureUniform;
        GLint       scaleInvertMatUniform;
        GLuint      normalTexture;
        GLint       normalUniform;
        GLuint      shaderProgram;
        glm::mat4   scaleMatrix;
    };

    struct TreeGLData
    {
        ui16    treeMeshIds[TREE_LOD_COUNT];        
        GLuint  treeShaderProgram;

        ImpostorGLData impostorData;
    };

    struct TreeGroup
    {
        glm::vec2 position;
        float radius;
        float spacing;
    };

    struct FrustrumCullingData
    {
        glm::vec4 leftPlane;
        glm::vec4 rightPlane;
        glm::vec4 bottomPlane;
        glm::vec4 topPlane;
        glm::vec4 nearPlane;
        glm::vec4 farPlane;
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
        float patchSize;
        float baseHeight;
        float heightScale;
        glm::mat4 worldToTerrainspace;

        ushort quadPatchIndices[4]; //indices for a quad, not 2 triangles
        glm::vec3 quadVertices[4];

        TerrainGLData   glData;
        TreeGLData      treeGlData;

        float *heightmap;
        std::vector<glm::mat4> patchModelMatrices;
        std::vector<glm::vec4> patchBoundingBoxCenters;

        std::vector<TreeGroup> treeGroups;
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

            for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
            {
                SCE::MeshRender::DeleteMeshRenderData(terrainData->treeGlData.treeMeshIds[lod]);
                SCE::MeshLoader::DeleteMesh(terrainData->treeGlData.treeMeshIds[lod]);
                terrainData->treeGlData.treeMeshIds[lod] = ui16(-1);
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

        //Spread tree groups over the terrain
        void initializeTreeLayout(glm::vec4* normAndHeightTex, float xOffset, float zOffset,
                                  float startScale, float heightScale,
                                  float halfTerrainSize)
        {
            //number of time the map is divided to form tree groups
            int treeGroupIter = 16;
            float scale = startScale;
            float baseGroupRadius = 1.0f / float(treeGroupIter)*halfTerrainSize;
            float maxRadiusScale = 3.0f;
            float baseSpacing = 40.0f;

            float y = 115.0f; //any value will do, just need to be something else than the terrain height

            for(int xCount = 0; xCount < treeGroupIter; ++xCount)
            {
                float x = float(xCount) / float(treeGroupIter);

                for(int zCount = 0; zCount < treeGroupIter; ++zCount)
                {
                    float z = float(zCount) / float(treeGroupIter);

                    vec4 normAndHeight = normAndHeightTex[int(x*TERRAIN_TEXTURE_SIZE) *
                            TERRAIN_TEXTURE_SIZE + int(z*TERRAIN_TEXTURE_SIZE)];

                    float noise = stb_perlin_noise3((x + xOffset)*scale, y*scale, (z + zOffset)*scale);
                    noise = SCE::Math::MapToRange(-0.7f, 0.7f, 0.0f, maxRadiusScale, noise);

                    float flatness = pow(dot(vec3(normAndHeight.x, normAndHeight.y, normAndHeight.z),
                                             vec3(0.0, 1.0, 0.0)), 8.0);
                    float height = normAndHeight.a / heightScale;
                    //Spawn tree at low height on flat terrain
                    if(height < 0.45f && flatness > 0.45f && noise > 0.6f)
                    {
                        TreeGroup group;
                        group.position = (glm::vec2(x, z)*2.0f - vec2(1.0, 1.0))
                               *halfTerrainSize;
                        group.radius = noise*baseGroupRadius;
                        group.spacing = baseSpacing;
                        terrainData->treeGroups.push_back(group);
                    }
                }
            }
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
                        float tmpNoise = stb_perlin_noise3(x*scale, y*scale, z*scale);
                        tmpNoise = SCE::Math::MapToRange(-0.7f, 0.7f, 0.0f, 1.0f, tmpNoise);
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

            initializeTreeLayout(packedNormalAndHeight, xOffset, zOffset, startScale, heightScale,
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

        FrustrumCullingData computeCullingData(const glm::mat4& projectionMatrix)
        {
            glm::mat4 invP = glm::inverse(projectionMatrix);
            FrustrumCullingData data;
            data.leftPlane = invP*vec4(1.0, 0.0, 0.0, 1.0);
            data.rightPlane = invP*vec4(-1.0, 0.0, 0.0, 1.0);
            data.bottomPlane = invP*vec4(0.0, 1.0, 0.0, 1.0);
            data.topPlane = invP*vec4(0.0, -1.0, 0.0, 1.0);
            data.nearPlane = invP*vec4(0.0, 0.0, 1.0, 1.0);
            data.farPlane = invP*vec4(0.0, 0.0, -1.0, 1.0);

            return data;
        }

// old frustrum culing function, fast but does not handle case where a quad is bigger than the screen
#if 0
        bool isQuadOffscreen(const glm::mat4& modelMatrix,
                             const glm::mat4& viewMatrix,
                             const glm::mat4& projectionMatrix)
        {
            int onScreenCount = 4;
            for(int i = 0; i < 4; ++i)
            {
                glm::vec4 pos_worldspace = modelMatrix*glm::vec4(terrainData->quadVertices[i], 1.0);
                float height = GetTerrainHeight(glm::vec3(pos_worldspace));
                pos_worldspace.y += height;
                glm::vec4 pos_viewspace = viewMatrix*pos_worldspace;
                glm::vec4 pos_screenspace = projectionMatrix*pos_viewspace;

                pos_screenspace /= pos_screenspace.w;
                float tolerance = 1.7f;
                if((abs(pos_screenspace.x) > tolerance || abs(pos_screenspace.y) > tolerance
                   || abs(pos_screenspace.z) > 1.00001f) && length(pos_screenspace) > 2.0f)
                {
                    --onScreenCount;
                }
            }

            return onScreenCount == 0;
        }
#endif

        bool isSphereInFrustrum(const FrustrumCullingData& cullingData,
                             const glm::vec4& pos_cameraspace,
                             float radius_cameraspace)
        {
            if(     glm::dot(cullingData.leftPlane, pos_cameraspace)   < -radius_cameraspace ||
                    glm::dot(cullingData.rightPlane, pos_cameraspace)  < -radius_cameraspace ||
                    glm::dot(cullingData.topPlane, pos_cameraspace)    < -radius_cameraspace ||
                    glm::dot(cullingData.bottomPlane, pos_cameraspace) < -radius_cameraspace ||
                    glm::dot(cullingData.nearPlane, pos_cameraspace)   < -radius_cameraspace ||
                    glm::dot(cullingData.farPlane, pos_cameraspace)    < -radius_cameraspace )
            {
                return false;
            }

            return true;
        }

        bool isBoxInFrustrum(const FrustrumCullingData& cullingData,
                             const glm::vec4& pos_cameraspace,
                             const glm::vec4& R_cameraspace,
                             const glm::vec4& S_cameraspace,
                             const glm::vec4& T_cameraspace)
        {
            glm::vec4 xVec = R_cameraspace;
            glm::vec4 yVec = S_cameraspace;
            glm::vec4 zVec = T_cameraspace;

            //compute effective radius in each frustrum direction
            float radiusProjLeft =
                    (glm::abs(glm::dot(cullingData.leftPlane, xVec)) +
                    glm::abs(glm::dot(cullingData.leftPlane, yVec)) +
                    glm::abs(glm::dot(cullingData.leftPlane, zVec)))*0.5;

            float radiusProjRight =
                    (glm::abs(glm::dot(cullingData.rightPlane, xVec)) +
                    glm::abs(glm::dot(cullingData.rightPlane, yVec)) +
                    glm::abs(glm::dot(cullingData.rightPlane, zVec)))*0.5;

            float radiusProjTop =
                    (glm::abs(glm::dot(cullingData.topPlane, xVec)) +
                    glm::abs(glm::dot(cullingData.topPlane, yVec)) +
                    glm::abs(glm::dot(cullingData.topPlane, zVec)))*0.5;

            float radiusProjBottom =
                    (glm::abs(glm::dot(cullingData.bottomPlane, xVec)) +
                    glm::abs(glm::dot(cullingData.bottomPlane, yVec)) +
                    glm::abs(glm::dot(cullingData.bottomPlane, zVec)))*0.5;

            float radiusProjNear =
                    (glm::abs(glm::dot(cullingData.nearPlane, xVec)) +
                    glm::abs(glm::dot(cullingData.nearPlane, yVec)) +
                    glm::abs(glm::dot(cullingData.nearPlane, zVec)))*0.5;

            float radiusProjFar =
                    (glm::abs(glm::dot(cullingData.farPlane, xVec)) +
                    glm::abs(glm::dot(cullingData.farPlane, yVec)) +
                    glm::abs(glm::dot(cullingData.farPlane, zVec)))*0.5;

            //for each plane, compare the distance from the (center, plane) to the effective radius
            //in this frustrum direction
            if(     glm::dot(cullingData.leftPlane, pos_cameraspace)   < -radiusProjLeft   ||
                    glm::dot(cullingData.rightPlane, pos_cameraspace)  < -radiusProjRight  ||
                    glm::dot(cullingData.topPlane, pos_cameraspace)    < -radiusProjTop    ||
                    glm::dot(cullingData.bottomPlane, pos_cameraspace) < -radiusProjBottom ||
                    glm::dot(cullingData.nearPlane, pos_cameraspace)   < -radiusProjNear   ||
                    glm::dot(cullingData.farPlane, pos_cameraspace)    < -radiusProjFar    )
            {
                return false;
            }

            return true;
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
            glData.terrainMaxDistanceUniform =
                    glGetUniformLocation(terrainProgram, TERRAIN_MAX_DIST_UNIFORM);
            glData.heightScaleUniform = glGetUniformLocation(terrainProgram, HEIGHT_SCALE_UNIFORM);
            glData.tesselationOverrideUniform =
                    glGetUniformLocation(terrainProgram, TESS_OVERRIDE_UNIFORM);

            glData.worldToTerrainMatUniform =
                    glGetUniformLocation(terrainProgram, WORLD_TO_TERRAIN_UNIFORM);

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

            //Load tree models
            TreeGLData& treeData = terrainData->treeGlData;
            for(int lod = 0; lod < TREE_LOD_COUNT; ++lod)
            {
                ui16 meshId = SCE::MeshLoader::CreateMeshFromFile(TREE_MODEL_NAME + std::to_string(lod)
                                                                  + ".obj");
                SCE::MeshRender::InitializeMeshRenderData(meshId);
                SCE::MeshRender::MakeMeshInstanced(meshId);
                treeData.treeMeshIds[lod] = meshId;
            }
            treeData.treeShaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_SHADER_NAME);

            //Set up impostor render data
            treeData.impostorData.meshId = SCE::MeshLoader::CreateQuadMesh();
            SCE::MeshRender::InitializeMeshRenderData(treeData.impostorData.meshId);
            SCE::MeshRender::MakeMeshInstanced(treeData.impostorData.meshId);
            treeData.impostorData.shaderProgram =
                    SCE::ShaderUtils::CreateShaderProgram(IMPOSTOR_SHADER_NAME);

            treeData.impostorData.texture = SCE::TextureUtils::LoadTexture(IMPOSTOR_TEXTURE_NAME);
            treeData.impostorData.normalTexture = SCE::TextureUtils::LoadTexture(IMPOSTOR_NORMAL_NAME);
            glBindTexture(GL_TEXTURE_2D, treeData.impostorData.texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            treeData.impostorData.textureUniform =
                    glGetUniformLocation(treeData.impostorData.shaderProgram, IMPOSTOR_TEXTURE_UNIFORM);
            treeData.impostorData.normalUniform =
                    glGetUniformLocation(treeData.impostorData.shaderProgram, IMPOSTOR_NORMAL_UNIFORM);
            treeData.impostorData.scaleInvertMatUniform =
                glGetUniformLocation(treeData.impostorData.shaderProgram, IMPOSTOR_SCALE_INVERT_UNIFORM);

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

        void spawnTreeInstances(const glm::mat4& viewMatrix,
                                const glm::mat4& worldToTerrainspaceMatrix,
                                const glm::vec3& cameraPosition,
                                const FrustrumCullingData& cullingData)
        {
            float perlinScale = 0.05f;
            float patchSize = terrainData->patchSize;
            float terrainSize = terrainData->terrainSize;
            //compute the actual terrain size we will cover with patches
            terrainSize = floor(terrainSize/patchSize)*patchSize;
            float halfTerrainSize = terrainSize / 2.0f;

            std::vector<glm::mat4> treeMatrices[TREE_LOD_COUNT];
            std::vector<glm::mat4> treeImpostorMatrices;
            float noiseX, noiseZ;

            //scale impostor quad so that it looks like a regular tree
            glm::mat4 impostorScaleMat =
                    terrainData->treeGlData.impostorData.scaleMatrix*
                    glm::scale(mat4(1.0), glm::vec3(IMPOSTOR_SIZE))*
                    glm::translate(mat4(1.0), glm::vec3(0.0, 1.0, 0.0));

            //apply a power to the distance to the tree to have the LOD group be exponentionally large
//            float power = 1.45f;
            float power = 1.6f;
            float maxDistToCam = 6000;

            int discardedGroups = 0;
            //Spawn trees from tree groups
            int lodGroup = 0;

            for(TreeGroup& group : terrainData->treeGroups)
            {
                //make a bigger radius to account for possible displacement
                float totalRadius = group.radius*2.0f;
                glm::vec3 groupPos = glm::vec3(group.position.x, 0.0f,
                                               group.position.y);
                groupPos.y = GetTerrainHeight(groupPos);
                glm::vec4 groupPos_cameraspace = viewMatrix*glm::vec4(groupPos, 1.0);

                if(isSphereInFrustrum(cullingData, groupPos_cameraspace, totalRadius))
                {
                    float distToCam = glm::max(0.0f, length(groupPos_cameraspace) - group.radius);
                    distToCam = pow(distToCam , power);
                    lodGroup = int(floor(distToCam/maxDistToCam));

                    for(float x = -group.radius; x < group.radius; x += group.spacing)
                    {
                        for(float z = -group.radius; z < group.radius; z += group.spacing)
                        {
                            //compute tree position from group and non-random noise
                            glm::vec3 treePos(x + group.position.x, 0.0f, z + group.position.y);
                            noiseX = stb_perlin_noise3(treePos.x*perlinScale, 25.0f,
                                                       treePos.z*perlinScale);
                            noiseZ = stb_perlin_noise3(treePos.z*perlinScale, 25.0f,
                                                       treePos.x*perlinScale);
                            //scale to apply to the tree
                            float scale = SCE::Math::MapToRange(-0.6f, 0.6f, 0.8f, 1.4f, noiseZ);

                            treePos.x += noiseX*group.spacing*2.0f;
                            treePos.z += noiseZ*group.spacing*2.0f;

                            //tree could have spawn outside of terrain, only keep if inside
                            if( abs(treePos.x) < halfTerrainSize - patchSize*0.5f
                                && abs(treePos.z) < halfTerrainSize - patchSize*0.5f)
                            {
                                //put tree at the surface of terrain
                                glm::vec4 terrainPos = worldToTerrainspaceMatrix*glm::vec4(treePos, 1.0);
                                int terrX = int((terrainPos.x*0.5 + 0.5)*TERRAIN_TEXTURE_SIZE);
                                int terrZ = int((terrainPos.z*0.5 + 0.5)*TERRAIN_TEXTURE_SIZE);
                                float height = terrainData->heightmap[terrX*TERRAIN_TEXTURE_SIZE + terrZ];
                                //go slightly down to avoid sticking out of the ground
                                treePos.y += height - 1.5f*scale;

                                glm::mat4 instanceMatrix;

                                //make a tree model
                                if(lodGroup < TREE_LOD_COUNT)
                                {
                                    lodGroup = clamp(lodGroup, 0, TREE_LOD_COUNT - 1);
                                    instanceMatrix = glm::translate(mat4(1.0f), treePos)*
                                            glm::rotate(mat4(1.0), noiseX*10.0f, glm::vec3(0.0, 1.0, 0.0))*
                                            glm::scale(mat4(1.0), glm::vec3(scale));
                                    treeMatrices[lodGroup].push_back(instanceMatrix);
                                }
                                //make an impostor
                                else
                                {
                                    //rotate plane to face camera
                                    glm::vec3 dirToCam = glm::normalize(cameraPosition - treePos);
                                    float angleYAxis = glm::atan(1.0f, 0.0f) -
                                            glm::atan(dirToCam.z, dirToCam.x);

                                    instanceMatrix = glm::translate(mat4(1.0f), treePos)*
                                            glm::rotate(mat4(1.0), angleYAxis, glm::vec3(0.0, 1.0, 0.0))*
                                            glm::scale(mat4(1.0), glm::vec3(scale))*
                                            impostorScaleMat;
                                    treeImpostorMatrices.push_back(instanceMatrix);
                                }
                            }
                        }
                    }
                }
                else
                {
                    ++discardedGroups;
                }
            }

            SCE::DebugText::Print("Tree groups : " + std::to_string(terrainData->treeGroups.size()));
            SCE::DebugText::Print("Tree groups skipped: " + std::to_string(discardedGroups));

            for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
            {
                SCE::MeshRender::SetMeshInstances(terrainData->treeGlData.treeMeshIds[lod],
                                                  treeMatrices[lod], GL_DYNAMIC_DRAW);
                SCE::DebugText::Print("Trees lod " + std::to_string(lod) + " : " +
                                      std::to_string(treeMatrices[lod].size()));
            }

            SCE::DebugText::Print("Trees impostors " +
                                  std::to_string(treeImpostorMatrices.size()));

            SCE::MeshRender::SetMeshInstances(terrainData->treeGlData.impostorData.meshId,
                                              treeImpostorMatrices, GL_DYNAMIC_DRAW);
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
        glm::vec3 cameraPosition = glm::vec3(glm::inverse(viewMatrix)*glm::vec4(0.0, 0.0, 0.0, 1.0));
        SCE::DebugText::Print("Cam : " + std::to_string(cameraPosition.x)
                              + ", " + std::to_string(cameraPosition.y)
                              + ", " + std::to_string(cameraPosition.z));

#ifdef DISPLAY_TREES
        FrustrumCullingData cullingData = computeCullingData(projectionMatrix);
        spawnTreeInstances(viewMatrix, terrainData->worldToTerrainspace,
                           cameraPosition, cullingData);
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

        if(isShadowPass > 0.0f)
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

        //uniforms
        glUniform1i(glData.terrainTextureUniform, 0);//terrain height map is sampler 0
        glUniform1f(glData.terrainMaxDistanceUniform, terrainData->terrainSize);
        glUniform1f(glData.heightScaleUniform, terrainData->heightScale);
        glUniform1f(glData.tesselationOverrideUniform, isShadowPass ? 8.0f : -1.0f);
        glUniform1f(glData.textureTileScaleUniform, terrainData->terrainSize / TEX_TILE_SIZE);

        glUniformMatrix4fv(glData.worldToTerrainMatUniform, 1, GL_FALSE,
                           &(terrainData->worldToTerrainspace[0][0]));

        glBindVertexArray(terrainData->quadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->quadIndicesVbo);

        FrustrumCullingData cullingData = computeCullingData(projectionMatrix);
        float patchSize = terrainData->patchSize;

        glm::vec4 R_cameraspace = viewMatrix*glm::vec4(patchSize, 0.0, 0.0, 0.0);
        glm::vec4 S_cameraspace = viewMatrix*glm::vec4(0.0, patchSize, 0.0, 0.0);
        glm::vec4 T_cameraspace = viewMatrix*glm::vec4(0.0, 0.0, patchSize, 0.0);

        uint patchCount = 0;
        uint offscreenCount = 0;
        for(uint i = 0; i < terrainData->patchModelMatrices.size(); ++i)
        {
            glm::vec4 center_cameraspace = viewMatrix*terrainData->patchBoundingBoxCenters[i];

            if(isBoxInFrustrum(cullingData, center_cameraspace, R_cameraspace,
                               S_cameraspace, T_cameraspace))
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

        SCE::DebugText::Print("Rendering terrain");
        SCE::DebugText::Print("Patches rendered : " + std::to_string(patchCount));
        SCE::DebugText::Print("Patches offscreen : " + std::to_string(offscreenCount));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //render trees
        TreeGLData& treeData = terrainData->treeGlData;
        glUseProgram(treeData.treeShaderProgram);

#ifdef DISPLAY_TREES

        for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
        {
            SCE::MeshRender::DrawInstances(treeData.treeMeshIds[lod], projectionMatrix, viewMatrix);
        }

        if(!isShadowPass)//impostors don't cast shadows
        {
            //render tree impostors
            glUseProgram(treeData.impostorData.shaderProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, treeData.impostorData.texture);
            glUniform1i(treeData.impostorData.textureUniform, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, treeData.impostorData.normalTexture);
            glUniform1i(treeData.impostorData.normalUniform, 1);

            glm::mat4 scaleInvert = glm::inverse(treeData.impostorData.scaleMatrix);
            glUniformMatrix4fv(treeData.impostorData.scaleInvertMatUniform, 1, GL_FALSE, &(scaleInvert[0][0]));

            SCE::MeshRender::DrawInstances(treeData.impostorData.meshId, projectionMatrix, viewMatrix);
        }
#endif

    }

    void Init(float terrainSize, float patchSize, float terrainBaseHeight)
    {
        if(!terrainData)
        {                        
            //compute the actual terrain size we will cover with patches
            terrainSize = floor(terrainSize/patchSize)*patchSize;
            float halfTerrainSize = terrainSize / 2.0f;

            terrainData = new TerrainData();
            terrainData->terrainSize = terrainSize;
            terrainData->patchSize = patchSize;
            terrainData->baseHeight = terrainBaseHeight;
            terrainData->heightScale = 1200.0f;            

            glm::vec3 terrainPosition_worldspace = vec3(0.0);
            terrainPosition_worldspace.y = terrainData->baseHeight;
            //create matrix to convert from woldspace to terrain space coord
            terrainData->worldToTerrainspace = glm::scale(mat4(1.0f), glm::vec3(1.0f / (halfTerrainSize)))
                    * glm::translate(mat4(1.0f), -terrainPosition_worldspace);

            initializeRenderData();
            float xOffset = SCE::Math::RandRange(0.0f, 1.0f);
            float zOffset = SCE::Math::RandRange(0.0f, 1.0f);
            initializeTerrain(xOffset, zOffset, 2.0f*terrainSize / 3000.0f,
                                      terrainData->heightScale);
            initializePatchMatrices();
        }
    }

    void Cleanup()
    {
        if(terrainData)
        {
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
