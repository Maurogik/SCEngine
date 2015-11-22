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

#include <stb_perlin.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#define TREE_SHADER_NAME "Terrain/Tree"
#define TREE_MODEL_NAME "Terrain/Meshes/tree_lod"

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
    treeGlData.shaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_SHADER_NAME);

    for(int lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        ui16 meshId = SCE::MeshLoader::CreateMeshFromFile(TREE_MODEL_NAME +
                                                          std::to_string(lod+TREE_LOD_MIN) +
                                                          ".obj");

        SCE::MeshRender::InitializeMeshRenderData(meshId);
        SCE::MeshRender::MakeMeshInstanced(meshId);
        treeGlData.meshIds[lod] = meshId;
    }

    //Set up impostor render data
    treeGlData.impostorData.meshId = SCE::MeshLoader::CreateQuadMesh();
    SCE::MeshRender::InitializeMeshRenderData(treeGlData.impostorData.meshId);
    SCE::MeshRender::MakeMeshInstanced(treeGlData.impostorData.meshId);
    treeGlData.impostorData.shaderProgram =
            SCE::ShaderUtils::CreateShaderProgram(IMPOSTOR_SHADER_NAME);

    treeGlData.impostorData.texture = SCE::TextureUtils::LoadTexture(IMPOSTOR_TEXTURE_NAME);
    treeGlData.impostorData.normalTexture = SCE::TextureUtils::LoadTexture(IMPOSTOR_NORMAL_NAME);
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
    if(treeGlData.shaderProgram != GLuint(-1))
    {
        SCE::ShaderUtils::DeleteShaderProgram(treeGlData.shaderProgram);
    }

    if(treeGlData.impostorData.shaderProgram != GLuint(-1))
    {
        SCE::ShaderUtils::DeleteShaderProgram(treeGlData.impostorData.shaderProgram);
    }

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::DeleteMeshRenderData(treeGlData.meshIds[lod]);
        SCE::MeshLoader::DeleteMesh(treeGlData.meshIds[lod]);
        treeGlData.meshIds[lod] = ui16(-1);
    }

    if(treeGlData.impostorData.texture != GLuint(-1))
    {
        SCE::TextureUtils::DeleteTexture(treeGlData.impostorData.texture);
    }

    if(treeGlData.impostorData.normalTexture != GLuint(-1))
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

            vec4 normAndHeight = normAndHeightTex[int(x*textureSize) *
                    textureSize + int(z*textureSize)];

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
    float power = 1.45f;
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

    SCE::DebugText::Print("Tree groups : " + std::to_string(treeGroups.size()));
    SCE::DebugText::Print("Tree groups skipped: " + std::to_string(discardedGroups));

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::SetMeshInstances(treeGlData.meshIds[lod],
                                          treeMatrices[lod], GL_DYNAMIC_DRAW);
        SCE::DebugText::Print("Trees lod " + std::to_string(lod) + " : " +
                              std::to_string(treeMatrices[lod].size()));
    }

    SCE::DebugText::Print("Trees impostors " +
                          std::to_string(treeImpostorMatrices.size()));

    SCE::MeshRender::SetMeshInstances(treeGlData.impostorData.meshId,
                                      treeImpostorMatrices, GL_DYNAMIC_DRAW);
}

void SCE::TerrainTrees::RenderTrees(const mat4 &projectionMatrix, const mat4 &viewMatrix,
                                    bool isShadowPass)
{
    //render trees
    glUseProgram(treeGlData.shaderProgram);

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::DrawInstances(treeGlData.meshIds[lod], projectionMatrix, viewMatrix);
    }

    if(!isShadowPass)//impostors don't cast shadows
    {
        //render tree impostors
        glUseProgram(treeGlData.impostorData.shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, treeGlData.impostorData.texture);
        glUniform1i(treeGlData.impostorData.textureUniform, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, treeGlData.impostorData.normalTexture);
        glUniform1i(treeGlData.impostorData.normalUniform, 1);

        glm::mat4 scaleInvert = glm::inverse(treeGlData.impostorData.scaleMatrix);
        glUniformMatrix4fv(treeGlData.impostorData.scaleInvertMatUniform, 1, GL_FALSE,
                           &(scaleInvert[0][0]));

        SCE::MeshRender::DrawInstances(treeGlData.impostorData.meshId, projectionMatrix, viewMatrix);
    }
}
