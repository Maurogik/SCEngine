/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCETerrainTrees.cpp*******/
/**************************************/


#include "../headers/SCETerrainTrees.hpp"
#include "../headers/SCEDebugText.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCEMeshRender.hpp"
#include "../headers/SCETextures.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCETerrain.hpp"
#include "../headers/SCEFrustrumCulling.hpp"
#include "../headers/SCEQuality.hpp"

#include <stb_perlin.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#define USE_STB_PERLIN 1
#if USE_STB_PERLIN
//#include <stb_perlin.h>
#else
#include "../headers/SCEPerlin.hpp"
#endif


#define TREE_TRUNK_SHADER_NAME "Terrain/TreePack/TreeInstanced_trunk"
#define TREE_LEAVES_SHADER_NAME "Terrain/TreePack/TreeInstanced_leaves"
#define TREE_MODEL_NAME "Terrain/TreePack/tree_1/1/lod"

#define TREE_MODEL_EXTENSION ".obj"

#define TREE_BARK_TEX_UNIFORM "BarkTex"
#define TREE_BARK_NORMAL_TEX_UNIFORM "BarkNormalMap"
#define TREE_LEAVES_TEX_UNIFORM "LeafTex"

#define IMPOSTOR_SHADER_NAME "Terrain/TreeImpostor"
#define IMPOSTOR_TEXTURE_UNIFORM "ImpostorTex"
#define IMPOSTOR_NORMAL_UNIFORM "ImpostorNormalTex"
#define IMPOSTOR_SCALE_INVERT_UNIFORM "ScaleInvertMat"
#define IMPOSTOR_TEXTURE_NAME "Terrain/Textures/treeImpostor"
#define IMPOSTOR_NORMAL_NAME "Terrain/Textures/treeImpostorNormal"
#define IMPOSTOR_SIZE 12.0f

//Spread tree groups over the terrain
SCE::TerrainTrees::TerrainTrees()
{
    treeGlData.impostorData.scaleMatrix = glm::scale(mat4(1.0), glm::vec3(0.5, 1.0, 1.0));

    //Load tree models
    treeGlData.trunkShaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_TRUNK_SHADER_NAME);
    treeGlData.leavesShaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_LEAVES_SHADER_NAME);

    for(int lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        std::string lodStr = std::to_string(lod + TREE_LOD_MIN);
        ui16 trunkMeshId = SCE::MeshLoader::CreateMeshFromFile(TREE_MODEL_NAME +
                                                          lodStr + "_trunk" +
                                                          TREE_MODEL_EXTENSION);
        SCE::MeshRender::InitializeMeshRenderData(trunkMeshId);
        SCE::MeshRender::MakeMeshInstanced(trunkMeshId);

        ui16 leavesMeshId = SCE::MeshLoader::CreateMeshFromFile(TREE_MODEL_NAME +
                                                          lodStr + "_leaves" +
                                                          TREE_MODEL_EXTENSION);
        SCE::MeshRender::InitializeMeshRenderData(leavesMeshId);
        SCE::MeshRender::MakeMeshInstanced(leavesMeshId);

        treeGlData.trunkMeshIds[lod] = trunkMeshId;
        treeGlData.leavesMeshIds[lod] = leavesMeshId;
    }

#if USE_PINE
    //load tree textures
    treeGlData.alphaTexture = SCE::TextureUtils::LoadTexture(TREE_ALPHA_TEXTURE_NAME);
    treeGlData.colorTexture = SCE::TextureUtils::LoadTexture(TREE_COLOR_TEXTURE_NAME);

    treeGlData.alphaTextureUniform =
            glGetUniformLocation(treeGlData.shaderProgram, TREE_ALPHA_TEX_UNIFORM);
    treeGlData.colorTextureUniform =
            glGetUniformLocation(treeGlData.shaderProgram, TREE_COLOR_TEX_UNIFORM);
#endif

    //Set up impostor render data
    treeGlData.impostorData.meshId = SCE::MeshLoader::CreateQuadMesh();
    SCE::MeshRender::InitializeMeshRenderData(treeGlData.impostorData.meshId);
    SCE::MeshRender::MakeMeshInstanced(treeGlData.impostorData.meshId);
    treeGlData.impostorData.shaderProgram = SCE::ShaderUtils::CreateShaderProgram(IMPOSTOR_SHADER_NAME);

    treeGlData.impostorData.texture         = SCE::TextureUtils::LoadTexture(IMPOSTOR_TEXTURE_NAME);
    treeGlData.impostorData.normalTexture   = SCE::TextureUtils::LoadTexture(IMPOSTOR_NORMAL_NAME);
    glBindTexture(GL_TEXTURE_2D, treeGlData.impostorData.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    treeGlData.impostorData.textureUniform =
            glGetUniformLocation(treeGlData.impostorData.shaderProgram, IMPOSTOR_TEXTURE_UNIFORM);
    treeGlData.impostorData.normalUniform =
            glGetUniformLocation(treeGlData.impostorData.shaderProgram, IMPOSTOR_NORMAL_UNIFORM);
    treeGlData.impostorData.scaleInvertMatUniform =
        glGetUniformLocation(treeGlData.impostorData.shaderProgram, IMPOSTOR_SCALE_INVERT_UNIFORM);    
}

SCE::TerrainTrees::~TerrainTrees()
{
    if(treeGlData.trunkShaderProgram != GL_INVALID_INDEX)
    {
        SCE::ShaderUtils::DeleteShaderProgram(treeGlData.trunkShaderProgram);
    }

    if(treeGlData.leavesShaderProgram != GL_INVALID_INDEX)
    {
        SCE::ShaderUtils::DeleteShaderProgram(treeGlData.leavesShaderProgram);
    }

    if(treeGlData.impostorData.shaderProgram != GL_INVALID_INDEX)
    {
        SCE::ShaderUtils::DeleteShaderProgram(treeGlData.impostorData.shaderProgram);
    }

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::DeleteMeshRenderData(treeGlData.trunkMeshIds[lod]);
        SCE::MeshLoader::DeleteMesh(treeGlData.trunkMeshIds[lod]);
        treeGlData.trunkMeshIds[lod] = ui16(-1);

        SCE::MeshRender::DeleteMeshRenderData(treeGlData.leavesMeshIds[lod]);
        SCE::MeshLoader::DeleteMesh(treeGlData.leavesMeshIds[lod]);
        treeGlData.leavesMeshIds[lod] = ui16(-1);
    }

    if(treeGlData.barkNormalTexture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(treeGlData.barkNormalTexture);
    }

    if(treeGlData.barkTexture!= GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(treeGlData.barkTexture);
    }

    if(treeGlData.leafTexture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(treeGlData.leafTexture);
    }

    if(treeGlData.impostorData.texture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(treeGlData.impostorData.texture);
    }

    if(treeGlData.impostorData.normalTexture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(treeGlData.impostorData.normalTexture);
    }
}

void SCE::TerrainTrees::InitializeTreeLayout(glm::vec4* normAndHeightTex, int textureSize,
                                             float xOffset, float zOffset,
                                             float startScale, float heightScale,
                                             float halfTerrainSize)
{
    //number of time the map is divided to form tree groups
    int treeGroupIter = int(32.0f*halfTerrainSize/1500.0f);
    float scale = startScale;
    float baseGroupRadius = (1.0f / float(treeGroupIter)) * halfTerrainSize;
    float maxRadiusScale = 4.0f;
    float baseSpacing = 40.0f;

    for(int xCount = 0; xCount < treeGroupIter; ++xCount)
    {
        float x = float(xCount) / float(treeGroupIter);

        for(int zCount = 0; zCount < treeGroupIter; ++zCount)
        {
            float z = float(zCount) / float(treeGroupIter);

            vec4 normAndHeight = normAndHeightTex[int(x*textureSize) *
                    textureSize + int(z*textureSize)];
#if USE_STB_PERLIN
            float y = 115.0f; //any value will do, just need to be something else than the terrain height
            float noise = stb_perlin_noise3((x + xOffset)*scale, y*scale, (z + zOffset)*scale);
            noise = SCE::Math::MapToRange(-0.7f, 0.7f, 0.0f, maxRadiusScale, noise);
#else
            float noise = Perlin::GetPerlinAt((x + xOffset)*scale, (z + zOffset)*scale);
            noise = SCE::Math::MapToRange(-0.5f, 0.5f, 0.0f, maxRadiusScale, noise);
#endif

            float flatness = pow(dot(vec3(normAndHeight.x, normAndHeight.y, normAndHeight.z),
                                     vec3(0.0, 1.0, 0.0)), 8.0);
            float height = normAndHeight.a / heightScale;
            //Spawn tree at low height on flat terrain
            if(height < 0.12f && flatness > 0.75f && noise > 0.4f * maxRadiusScale)
            {
                TreeGroup group;
                group.position = (glm::vec2(x, z)*2.0f - vec2(1.0, 1.0))
                       *halfTerrainSize;
                group.radius = noise*baseGroupRadius;
                group.spacing = baseSpacing;
                treeGroups.push_back(group);
            }
        }
    }
}

void SCE::TerrainTrees::SpawnTreeInstances(const glm::mat4& viewMatrix,
                                           const glm::mat4& worldToTerrainspaceMatrix,
                                           const glm::vec3& cameraPosition,
                                           float maxDistFromCenter)
{
    float perlinScale = 0.05f;

    std::vector<glm::mat4> treeMatrices[TREE_LOD_COUNT];
    std::vector<glm::mat4> treeImpostorMatrices;
    float noiseX, noiseZ;

    //scale impostor quad so that it looks like a regular tree
    glm::mat4 impostorScaleMat =
            treeGlData.impostorData.scaleMatrix*
            glm::scale(mat4(1.0), glm::vec3(IMPOSTOR_SIZE))*
            glm::translate(mat4(1.0), glm::vec3(0.0, 1.0, 0.0));

    //apply a power to the distance to the tree to have the LOD group be exponentionally large
    float power = 1.6f;
    float maxDistToCam = 4500;

    int discardedGroups = 0;
    //Spawn trees from tree groups
    int lodGroup = 0;

    for(TreeGroup& group : treeGroups)
    {
        //make a bigger radius to account for possible displacement
        float totalRadius = group.radius*2.0f;
        glm::vec3 groupPos = glm::vec3(group.position.x, 0.0f,
                                       group.position.y);
        groupPos.y = SCE::Terrain::GetTerrainHeight(groupPos);
        glm::vec4 groupPos_cameraspace = viewMatrix*glm::vec4(groupPos, 1.0);

        if(SCE::FrustrumCulling::IsSphereInFrustrum(groupPos_cameraspace, totalRadius))
        {
            float distToCam = glm::max(0.0f, length(groupPos_cameraspace) - group.radius);
            distToCam = pow(distToCam , power) * SCE::Quality::TreeLodMultiplier;
            lodGroup = int(floor(distToCam/maxDistToCam));

            for(float x = -group.radius; x < group.radius; x += group.spacing)
            {
                for(float z = -group.radius; z < group.radius; z += group.spacing)
                {
                    //compute tree position from group and non-random noise
                    glm::vec3 treePos(x + group.position.x, 0.0f, z + group.position.y);

#if USE_STB_PERLIN
                    noiseX = stb_perlin_noise3(treePos.x*perlinScale, 25.0f,
                                               treePos.z*perlinScale);
                    noiseZ = stb_perlin_noise3(treePos.z*perlinScale, 25.0f,
                                               treePos.x*perlinScale);
                    //scale to apply to the tree
                    float scale = SCE::Math::MapToRange(-0.6f, 0.6f, 1.0f, 1.8f, noiseZ);
#else
                    noiseX = Perlin::GetPerlinAt(treePos.x*perlinScale, treePos.z*perlinScale);
                    noiseZ = Perlin::GetPerlinAt(treePos.z*perlinScale, treePos.x*perlinScale);
                    //scale to apply to the tree
                    float scale = SCE::Math::MapToRange(-0.5f, 0.5f, 0.8f, 1.4f, noiseZ);
#endif

                    treePos.x += noiseX*group.spacing*5.0f;
                    treePos.z += noiseZ*group.spacing*5.0f;

                    //tree could have spawn outside of terrain, only keep if inside
                    if( abs(treePos.x) < maxDistFromCenter
                        && abs(treePos.z) < maxDistFromCenter)
                    {
                        //put tree at the surface of terrain
                        float height = SCE::Terrain::GetTerrainHeight(treePos);
                        //go slightly down to avoid sticking out of the ground
                        treePos.y += height - 1.5f*scale;

                        glm::mat4 instanceMatrix;

                        //make a tree model
                        if(lodGroup < TREE_LOD_COUNT)
                        {
                            lodGroup = clamp(lodGroup, 0, TREE_LOD_COUNT - 1);
                            instanceMatrix = glm::translate(mat4(1.0f), treePos)*
                                    glm::rotate(mat4(1.0), noiseX*10.0f, glm::vec3(0, 1, 0))*
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

    SCE::DebugText::LogMessage("Tree groups : " + std::to_string(treeGroups.size()));
    SCE::DebugText::LogMessage("Tree groups skipped: " + std::to_string(discardedGroups));

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::SetMeshInstances(treeGlData.trunkMeshIds[lod],
                                          treeMatrices[lod], GL_DYNAMIC_DRAW);
        SCE::MeshRender::SetMeshInstances(treeGlData.leavesMeshIds[lod],
                                          treeMatrices[lod], GL_DYNAMIC_DRAW);
        SCE::DebugText::LogMessage("Trees lod " + std::to_string(lod) + " : " +
                              std::to_string(treeMatrices[lod].size()));
    }

    SCE::DebugText::LogMessage("Trees impostors " +
                          std::to_string(treeImpostorMatrices.size()));

    SCE::MeshRender::SetMeshInstances(treeGlData.impostorData.meshId,
                                      treeImpostorMatrices, GL_DYNAMIC_DRAW);
}

void SCE::TerrainTrees::RenderTrees(const mat4 &projectionMatrix, const mat4 &viewMatrix,
                                    bool isShadowPass)
{
    //render trees trunks
    SCE::ShaderUtils::UseShader(treeGlData.trunkShaderProgram);

    //SCE::TextureUtils::BindTexture(treeGlData.barkTexture, 0, treeGlData.barkTexUniform);
    //SCE::TextureUtils::BindTexture(treeGlData.barkNormalTexture, 1, treeGlData.barkNormalTexUniform);

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::DrawInstances(treeGlData.trunkMeshIds[lod], projectionMatrix, viewMatrix);
    }

    //render trees leaves
    SCE::ShaderUtils::UseShader(treeGlData.leavesShaderProgram);

    //SCE::TextureUtils::BindTexture(treeGlData.leafTexture, 0, treeGlData.leafTexUniform);

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::DrawInstances(treeGlData.leavesMeshIds[lod], projectionMatrix, viewMatrix);
    }

    if(!isShadowPass)//impostors don't cast shadows
    {
        //render tree impostors
        SCE::ShaderUtils::UseShader(treeGlData.impostorData.shaderProgram);
        SCE::TextureUtils::BindTexture(treeGlData.impostorData.texture, 0,
                                       treeGlData.impostorData.textureUniform);
        SCE::TextureUtils::BindTexture(treeGlData.impostorData.normalTexture, 1,
                                       treeGlData.impostorData.normalUniform);

        glm::mat4 scaleInvert = glm::inverse(treeGlData.impostorData.scaleMatrix);
        glUniformMatrix4fv(treeGlData.impostorData.scaleInvertMatUniform, 1, GL_FALSE,
                           &(scaleInvert[0][0]));

        SCE::MeshRender::DrawInstances(treeGlData.impostorData.meshId, projectionMatrix, viewMatrix);
    }
}
