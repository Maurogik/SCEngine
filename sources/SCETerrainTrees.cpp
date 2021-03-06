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
#include "../headers/SCETime.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCEBillboardRender.hpp"
#include "../headers/SCEScene.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#define USE_STB_PERLIN 1
#if USE_STB_PERLIN
    #ifndef STB_PERLIN_IMPLEMENTATION
    #define STB_PERLIN_IMPLEMENTATION
    #include <stb_perlin.h>
    #endif
#else
#include "../headers/SCEPerlin.hpp"
#endif

#define USE_IMPOSTORS 1
#define DOUBLE_SIDED_TREES 1
#define NB_IMPOSTOR_ANGLES (6*6)

#define USE_THREADED_UPDATE 1

#define IMPOSTOR_FLIP_X 0
#define IMPOSTOR_FACE_Z 0

#define TREE_TRUNK_SHADER_NAME "Terrain/TreePack/TreeInstanced_trunk"
#define TREE_LEAVES_SHADER_NAME "Terrain/TreePack/TreeInstanced_leaves"
#define TREE_MODEL_NAME "Terrain/TreePack/tree_5/1/lod"

#define TREE_MODEL_EXTENSION ".obj"

#define TREE_BARK_TEX_NAME "Terrain/TreePack/tree_1/bark.png"
#define TREE_BARK_NORMAL_NAME "Terrain/TreePack/tree_5/bark_normal.png"
#define TREE_LEAVES_TEX_NAME "Terrain/TreePack/tree_5/leafs.png"
#define TREE_BARK_TEX_UNIFORM "BarkTex"
#define TREE_BARK_NORMAL_TEX_UNIFORM "BarkNormalMap"
#define TREE_LEAVES_TEX_UNIFORM "LeafTex"
#define TREE_LEAVES_TRANSLUCENCY_UNIFORM "Translucency"
#define TREE_LEAVES_TRANSLUCENCY 0.5f

#define BILLBOARD_GEN_SHADER_NAME "BillboardCapture"
#define BILLBOARD_GEN_MAIN_TEX "MainTex"
#define BILLBOARD_GEN_NORMAL_TEX "NormalMap"
#define BILLBOARD_GEN_TRANSLUCENCY "Translucency"
#define BILLBOARD_BORDER 0.05f

#define IMPOSTOR_SHADER_NAME "Terrain/TreeImpostor"
#define IMPOSTOR_TEXTURE_UNIFORM "ImpostorTex"
#define IMPOSTOR_NORMAL_UNIFORM "ImpostorNormalTex"

void SCE::TerrainTrees::UpdateVisibilityAndLOD(mat4 viewMatrix,
                                               mat4 worldToTerrainspaceMatrix,
                                               vec3 cameraPosition_scenespace,
                                               float maxDistFromCenter,
                                               glm::mat4 impostorScaleMat)
{
#if USE_THREADED_UPDATE

#define DO_SLEEP 0
#define DO_DURATION 1

#else

#define DO_SLEEP 0
#define DO_DURATION 0

#endif

#if DO_SLEEP
    std::chrono::milliseconds waitInterval(8);
#endif

#if DO_DURATION
    double startTime = SCE::Time::RealTimeInSeconds();
#endif

    float perlinScale = 0.05f;
    float positionNoiseScale = 5.0f;
    float noiseX, noiseZ;

    int discardedGroups = 0;
    int prevSize = 0;

    for(int i = 0; i < TREE_LOD_COUNT; ++i)
    {
        prevSize = mTreeMatrices[i].size();
        mTreeMatrices[i].clear();
        mTreeMatrices[i].reserve(prevSize);
    }

    prevSize = mTreeImpostorMatrices.size();
    mTreeImpostorMatrices.clear();
    mTreeImpostorMatrices.reserve(prevSize);
    mTreeImpostorTexMapping.clear();
    mTreeImpostorTexMapping.reserve(prevSize);

    //perform frustum culling on the tree groups
    std::vector<TreeGroup*> activeGroups;
    for(TreeGroup& group : mTreeGroups)
    {
        //make a bigger radius to account for possible displacement
        float totalRadius = group.radius + group.spacing*positionNoiseScale;
        glm::vec3 groupPos = glm::vec3(group.position.x, 0.0f,
                                       group.position.y);        
        groupPos.y = SCE::Terrain::GetTerrainHeight(groupPos);
        //convert to scenespace
        groupPos -= SCEScene::GetFrameRootPosition();

        glm::vec4 groupPos_cameraspace = viewMatrix*glm::vec4(groupPos, 1.0);

        if(SCE::FrustrumCulling::IsSphereInFrustrum(groupPos_cameraspace, totalRadius))
        {
            activeGroups.push_back(&group);
        }
        else
        {
            ++discardedGroups;
        }       
    }

#if DO_SLEEP
    std::this_thread::sleep_for(waitInterval);
#endif

    glm::vec3 camPosition_worldspace = SCEScene::GetFrameRootPosition() + cameraPosition_scenespace;
    glm::vec2 camPos2 = glm::vec2(camPosition_worldspace.x, camPosition_worldspace.z);

    //sort the visible tree groups
    std::sort(begin(activeGroups), end(activeGroups),
              [&camPos2](TreeGroup const* a, TreeGroup const* b) -> bool
    {
        return glm::length(camPos2 - a->position) < glm::length(camPos2 - b->position);
    });

#if DO_SLEEP
    std::this_thread::sleep_for(waitInterval);
#endif

    glm::vec3 leveledCamPos = camPosition_worldspace;
    leveledCamPos.y = 0.0f;
    //Spawn trees from tree groups
    int lodGroup = 0;

    for(TreeGroup* group : activeGroups)
    {
        for(float x = -group->radius; x < group->radius; x += group->spacing)
        {
            for(float z = -group->radius; z < group->radius; z += group->spacing)
            {
                //compute tree position from group and non-random noise
                glm::vec3 treePos(x + group->position.x, 0.0f, z + group->position.y);

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

                treePos.x += noiseX*group->spacing*positionNoiseScale;
                treePos.z += noiseZ*group->spacing*positionNoiseScale;

                float distToCam = glm::max(0.0f, length(treePos - leveledCamPos));
                for(lodGroup = 0; lodGroup < TREE_LOD_COUNT; ++lodGroup)
                {
                    if(distToCam < SCE::Quality::Trees::LodDistances[lodGroup])
                    {
                        break;
                    }
                }

                //tree could have spawn outside of terrain, only keep if inside
                if( abs(treePos.x) < maxDistFromCenter
                    && abs(treePos.z) < maxDistFromCenter
                    && distToCam < SCE::Quality::Trees::MaxDrawDistance)
                {
                    //put tree at the surface of terrain
                    float height = SCE::Terrain::GetTerrainHeight(treePos);
                    //go slightly down to avoid sticking out of the ground
                    treePos.y = height - 2.0f*scale;

                    glm::mat4 instanceMatrix;

                    //make a tree model
                    if(lodGroup < TREE_LOD_COUNT)
                    {
                        lodGroup = clamp(lodGroup, 0, TREE_LOD_COUNT - 1);
                        instanceMatrix = glm::translate(mat4(1.0f), treePos)*
                                glm::rotate(mat4(1.0), noiseX*10.0f, glm::vec3(0, 1, 0))*
                                glm::scale(mat4(1.0), glm::vec3(scale));
                        mTreeMatrices[lodGroup].push_back(instanceMatrix);
                    }
#if USE_IMPOSTORS
                    //make an impostor
                    else
                    {
                        //rotate plane to face camera
                        glm::vec3 dirToCam = glm::normalize(camPosition_worldspace - treePos);
#if IMPOSTOR_FACE_Z
                        float angleYAxis = glm::atan(1.0f, 0.0f) -
                                glm::atan(dirToCam.z, dirToCam.x);

                        instanceMatrix = glm::translate(mat4(1.0f), treePos)*
                                glm::rotate(mat4(1.0), angleYAxis, glm::vec3(0.0, 1.0, 0.0))*
                                glm::scale(mat4(1.0), glm::vec3(scale))*
                                impostorScaleMat;
                        mTreeImpostorMatrices.push_back(instanceMatrix);

                        mTreeImpostorTexMapping.push_back(
                                    SCE::BillboardRender::GetMappingForAgnle(noiseX*10.0f-angleYAxis,
                                                                             NB_IMPOSTOR_ANGLES,
                                                                             IMPOSTOR_FLIP_X));
#else
                        float angleYAxis = glm::atan(-1.0f, 0.0f) -
                                glm::atan(dirToCam.z, dirToCam.x);

                        instanceMatrix = glm::translate(mat4(1.0f), treePos)*
                                glm::rotate(mat4(1.0), angleYAxis, glm::vec3(0.0, 1.0, 0.0))*
                                glm::scale(mat4(1.0), glm::vec3(scale))*
                                impostorScaleMat;
                        mTreeImpostorMatrices.push_back(instanceMatrix);

                        mTreeImpostorTexMapping.push_back(
                                    SCE::BillboardRender::GetMappingForAgnle(noiseX*-10.0f+angleYAxis,
                                                                             NB_IMPOSTOR_ANGLES,
                                                                             IMPOSTOR_FLIP_X, BILLBOARD_BORDER));
#endif
                    }
#endif
                }
            }
        }
    }

#if DO_SLEEP
    std::this_thread::sleep_for(waitInterval);
#endif

#if DO_DURATION
    double ellapsedTime = SCE::Time::RealTimeInSeconds() - startTime;
    double remainingTime = SCE::Quality::Trees::VisibilityUpdateDuration - ellapsedTime;
    if(remainingTime > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(int(remainingTime*1000.0f)));
    }
#endif

    //sort non-impostors trees individually
    for(int i = 0; i < TREE_LOD_COUNT; ++i)
    {
        //sort the visible trees
        std::sort(begin(mTreeMatrices[i]), end(mTreeMatrices[i]),
                  [&leveledCamPos](glm::mat4 const& a, glm::mat4 const& b) -> bool
        {
            glm::vec3 aPos(a[3].x, 0.0f, a[3].z);
            glm::vec3 bPos(b[3].x, 0.0f, b[3].z);
            return length(leveledCamPos - aPos) < length(leveledCamPos - bPos);
        });

    }

    {
        mTreeInstanceLock.lock();
        mInstancesUpToDate = false;
        mTreeInstanceLock.unlock();
    }
}

//Spread tree groups over the terrain
SCE::TerrainTrees::TerrainTrees()
    : mInstancesUpToDate(false)
{
    //Load tree models
    mTreeGlData.trunkShaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_TRUNK_SHADER_NAME);
    mTreeGlData.leavesShaderProgram = SCE::ShaderUtils::CreateShaderProgram(TREE_LEAVES_SHADER_NAME);

    for(int lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        std::string lodStr = std::to_string(lod);
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

        mTreeGlData.trunkMeshIds[lod] = trunkMeshId;
        mTreeGlData.leavesMeshIds[lod] = leavesMeshId;
    }

    mTreeGlData.barkTexture = SCE::TextureUtils::LoadTexture(TREE_BARK_TEX_NAME);
    mTreeGlData.barkTexUniform =
            glGetUniformLocation(mTreeGlData.trunkShaderProgram, TREE_BARK_TEX_UNIFORM);

    mTreeGlData.barkNormalTexture = SCE::TextureUtils::LoadTexture(TREE_BARK_NORMAL_NAME);
    mTreeGlData.barkNormalTexUniform =
            glGetUniformLocation(mTreeGlData.trunkShaderProgram, TREE_BARK_NORMAL_TEX_UNIFORM);

    mTreeGlData.leafTexture = SCE::TextureUtils::LoadTexture(TREE_LEAVES_TEX_NAME);
    mTreeGlData.leafTexUniform =
            glGetUniformLocation(mTreeGlData.leavesShaderProgram, TREE_LEAVES_TEX_UNIFORM);
    mTreeGlData.leavesTranslucencyUniform =
            glGetUniformLocation(mTreeGlData.leavesShaderProgram, TREE_LEAVES_TRANSLUCENCY_UNIFORM);


#if USE_IMPOSTORS

    //generate textures for different angles
//    ui16 impostorSrcLeavesMesh = mTreeGlData.leavesMeshIds[TREE_LOD_COUNT - 1];
//    ui16 impostorSrcTrunkMesh = mTreeGlData.trunkMeshIds[TREE_LOD_COUNT - 1];
    ui16 impostorSrcLeavesMesh = mTreeGlData.leavesMeshIds[0];
    ui16 impostorSrcTrunkMesh = mTreeGlData.trunkMeshIds[0];

    GLuint billboardCaptureShader = SCE::ShaderUtils::CreateShaderProgram(BILLBOARD_GEN_SHADER_NAME);
    GLuint defaultNormalTex = SCE::TextureUtils::CreateTexture(64, 64, glm::vec4(0.0, 0.0, 1.0, 0.0));

    GLint billboardDiffuseUniform = glGetUniformLocation(billboardCaptureShader, BILLBOARD_GEN_MAIN_TEX);
    GLint billboardNormalUniform = glGetUniformLocation(billboardCaptureShader, BILLBOARD_GEN_NORMAL_TEX);
    GLint billboardTranslucencyUniform = glGetUniformLocation(billboardCaptureShader, BILLBOARD_GEN_TRANSLUCENCY);

    float trunkTransluceny = 0.0f;//put some translu on the trunk to avoid it going to dark

    //callback that will be call to render each angle
    SCE::BillboardRender::RenderCallback renderCallback = [&](glm::mat4 const& modelMatrix,
            glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix)
    {
        //render trees trunks
        SCE::ShaderUtils::UseShader(billboardCaptureShader);
        SCE::TextureUtils::BindTexture(mTreeGlData.barkTexture, 0, billboardDiffuseUniform);
        SCE::TextureUtils::BindTexture(mTreeGlData.barkNormalTexture, 1, billboardNormalUniform);
        glUniform1f(billboardTranslucencyUniform, trunkTransluceny);
        SCE::MeshRender::RenderMesh(impostorSrcTrunkMesh, projectionMatrix, viewMatrix, modelMatrix);

        //render trees leaves
        SCE::ShaderUtils::UseShader(billboardCaptureShader);
        SCE::TextureUtils::BindTexture(mTreeGlData.leafTexture, 0, billboardDiffuseUniform);
        SCE::TextureUtils::BindTexture(defaultNormalTex, 1, billboardNormalUniform);
        glUniform1f(billboardTranslucencyUniform, TREE_LEAVES_TRANSLUCENCY);
        SCE::MeshRender::RenderMesh(impostorSrcLeavesMesh, projectionMatrix, viewMatrix, modelMatrix);
    };

    glm::vec3 leavesDim = SCE::MeshLoader::GetMeshData(impostorSrcLeavesMesh).dimensions;
    glm::vec3 trunkDim = SCE::MeshLoader::GetMeshData(impostorSrcTrunkMesh).dimensions;
    glm::vec3 leavesCenter = SCE::MeshLoader::GetMeshData(impostorSrcLeavesMesh).center;
    glm::vec3 trunkCenter = SCE::MeshLoader::GetMeshData(impostorSrcTrunkMesh).center;


    glm::vec3 treeDimensions, treeCenter;
    SCE::Math::CombineAABB(leavesCenter, leavesDim, trunkCenter, trunkDim, treeCenter, treeDimensions);

    //Start generating the billboards
    glm::vec3 billboardSize = SCE::BillboardRender::GenerateTexturesFromMesh(
                                                   NB_IMPOSTOR_ANGLES, 512, BILLBOARD_BORDER,
                                                   treeCenter, treeDimensions,
                                                   &mTreeGlData.impostorData.texture,
                                                   &mTreeGlData.impostorData.normalTexture,
                                                   renderCallback);
    //we're done with this shader so delete it
    SCE::ShaderUtils::DeleteShaderProgram(billboardCaptureShader);
    billboardCaptureShader = GL_INVALID_INDEX;  

    //Set up impostor render data
    mTreeGlData.impostorData.meshId = SCE::MeshLoader::CreateQuadMesh("BillboardQuad", IMPOSTOR_FACE_Z);
    SCE::MeshRender::InitializeMeshRenderData(mTreeGlData.impostorData.meshId);
    SCE::MeshRender::MakeMeshInstanced(mTreeGlData.impostorData.meshId);
    mTreeGlData.impostorData.shaderProgram = SCE::ShaderUtils::CreateShaderProgram(IMPOSTOR_SHADER_NAME);

    mTreeGlData.impostorData.textureUniform =
            glGetUniformLocation(mTreeGlData.impostorData.shaderProgram, IMPOSTOR_TEXTURE_UNIFORM);
    mTreeGlData.impostorData.normalUniform =
            glGetUniformLocation(mTreeGlData.impostorData.shaderProgram, IMPOSTOR_NORMAL_UNIFORM);


    //scale impostor quad so that it looks like a regular tree
    mImpostorScaleMat =
            glm::scale(mat4(1.0), billboardSize + BILLBOARD_BORDER)*
            glm::translate(mat4(1.0), glm::vec3(0.0, 1.0, 0.0));

#endif

}

SCE::TerrainTrees::~TerrainTrees()
{
    if(mUpdateThread)
    {
        mUpdateThread->join();
        mUpdateThread.release();
    }

    if(mTreeGlData.trunkShaderProgram != GL_INVALID_INDEX)
    {
        SCE::ShaderUtils::DeleteShaderProgram(mTreeGlData.trunkShaderProgram);
    }

    if(mTreeGlData.leavesShaderProgram != GL_INVALID_INDEX)
    {
        SCE::ShaderUtils::DeleteShaderProgram(mTreeGlData.leavesShaderProgram);
    }

    if(mTreeGlData.impostorData.shaderProgram != GL_INVALID_INDEX)
    {
        SCE::ShaderUtils::DeleteShaderProgram(mTreeGlData.impostorData.shaderProgram);
    }

    for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
    {
        SCE::MeshRender::DeleteMeshRenderData(mTreeGlData.trunkMeshIds[lod]);
        SCE::MeshLoader::DeleteMesh(mTreeGlData.trunkMeshIds[lod]);
        mTreeGlData.trunkMeshIds[lod] = ui16(-1);

        SCE::MeshRender::DeleteMeshRenderData(mTreeGlData.leavesMeshIds[lod]);
        SCE::MeshLoader::DeleteMesh(mTreeGlData.leavesMeshIds[lod]);
        mTreeGlData.leavesMeshIds[lod] = ui16(-1);
    }

    if(mTreeGlData.barkNormalTexture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(mTreeGlData.barkNormalTexture);
    }

    if(mTreeGlData.barkTexture!= GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(mTreeGlData.barkTexture);
    }

    if(mTreeGlData.leafTexture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(mTreeGlData.leafTexture);
    }

    if(mTreeGlData.impostorData.texture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(mTreeGlData.impostorData.texture);
    }

    if(mTreeGlData.impostorData.normalTexture != GL_INVALID_INDEX)
    {
        SCE::TextureUtils::DeleteTexture(mTreeGlData.impostorData.normalTexture);
    }
}

void SCE::TerrainTrees::InitializeTreeLayout(float xOffset, float zOffset,
                                             float startScale, float heightScale,
                                             float halfTerrainSize)
{
    //number of time the map is divided to form tree groups
    float divPerKm = SCE::Quality::Trees::NbGroupPerKm;
    int treeGroupIter = int(divPerKm*halfTerrainSize/1000.0f);
    float scale = startScale;
    float baseGroupRadius = (1000.0f/divPerKm);
    float maxRadiusScale = 1.3f;
    float minRadiusScale = 0.7f;
    float baseSpacing = SCE::Quality::Trees::BaseSpacing;

    for(int xCount = 0; xCount < treeGroupIter; ++xCount)
    {
        float x = float(xCount) / float(treeGroupIter);

        for(int zCount = 0; zCount < treeGroupIter; ++zCount)
        {
            float z = float(zCount) / float(treeGroupIter);

#if USE_STB_PERLIN
            float y = 115.0f; //any value will do, just need to be something other than the terrain height
            float noise = stb_perlin_noise3((x + xOffset)*scale, y*scale, (z + zOffset)*scale);
            noise = SCE::Math::MapToRange(-0.7f, 0.7f, minRadiusScale, maxRadiusScale, noise);
#else
            float noise = Perlin::GetPerlinAt((x + xOffset)*scale, (z + zOffset)*scale);
            noise = SCE::Math::MapToRange(-0.5f, 0.5f, minRadiusScale, maxRadiusScale, noise);
#endif
            glm::vec2 pos = (glm::vec2(x, z)*2.0f - vec2(1.0, 1.0))*halfTerrainSize;

            glm::vec3 normal = SCE::Terrain::GetTerrainNormal(glm::vec3(pos.x, 0.0f, pos.y));
            float height = SCE::Terrain::GetTerrainHeight(glm::vec3(pos.x, 0.0f, pos.y))/heightScale;

            float flatness = pow(dot(normal, vec3(0.0f, 1.0f, 0.0f)), 8.0f);
            //Spawn tree at low height on flat terrain
            if(height < 0.3f && flatness > 0.6f
               && (noise - minRadiusScale) > (0.2f*(maxRadiusScale-minRadiusScale)))
            {
                TreeGroup group;
                group.position = pos;
                group.radius = noise*baseGroupRadius;
                group.spacing = baseSpacing;
                mTreeGroups.push_back(group);
            }
        }
    }
}

void SCE::TerrainTrees::SpawnTreeInstances(const glm::mat4& viewMatrix,
                                           const glm::mat4& worldToTerrainspaceMatrix,
                                           const glm::vec3& cameraPosition,
                                           float maxDistFromCenter)
{
    bool isUpToDate = false;
#if USE_THREADED_UPDATE
    mTreeInstanceLock.lock();
    isUpToDate = mInstancesUpToDate;
    mTreeInstanceLock.unlock();

    if(!isUpToDate && mUpdateThread)
    {
        mUpdateThread->join();
        mUpdateThread.release();
    }
#else
    SCE::TerrainTrees::UpdateVisibilityAndLOD(
                            viewMatrix,
                            worldToTerrainspaceMatrix,
                            cameraPosition,
                            maxDistFromCenter,
                            mImpostorScaleMat);
#endif

    if(!isUpToDate)
    {
        mTreeInstanceLock.lock();
        SCE::DebugText::LogMessage("Tree groups : " + std::to_string(mTreeGroups.size()));

        for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
        {
            SCE::MeshRender::SetMeshInstanceMatrices(mTreeGlData.trunkMeshIds[lod],
                                              mTreeMatrices[lod], GL_DYNAMIC_DRAW);
            SCE::MeshRender::SetMeshInstanceMatrices(mTreeGlData.leavesMeshIds[lod],
                                              mTreeMatrices[lod], GL_DYNAMIC_DRAW);
            SCE::DebugText::LogMessage("Trees lod " + std::to_string(lod) + " : " +
                                  std::to_string(mTreeMatrices[lod].size()));
        }

#if USE_IMPOSTORS
        SCE::DebugText::LogMessage("Trees impostors " +
                              std::to_string(mTreeImpostorMatrices.size()));

        SCE::MeshRender::SetMeshInstanceMatrices(mTreeGlData.impostorData.meshId,
                                          mTreeImpostorMatrices, GL_DYNAMIC_DRAW);
        SCE::MeshRender::SetInstanceCustomData(
                    mTreeGlData.impostorData.meshId,
                    mTreeImpostorTexMapping.data(),
                    mTreeImpostorTexMapping.size()*sizeof(mTreeImpostorTexMapping[0]),
                    GL_DYNAMIC_DRAW,
                    4,
                    GL_FLOAT);
#endif
        mInstancesUpToDate = true;

        mTreeInstanceLock.unlock();

#if USE_THREADED_UPDATE
        mUpdateThread.reset(new std::thread(&SCE::TerrainTrees::UpdateVisibilityAndLOD,
                                            this,
                                            viewMatrix,
                                            worldToTerrainspaceMatrix,
                                            cameraPosition,
                                            maxDistFromCenter,
                                            mImpostorScaleMat));
#endif
    }
}

void SCE::TerrainTrees::RenderTrees(const mat4 &projectionMatrix, const mat4 &viewMatrix,
                                    bool isShadowPass)
{
    {
        mTreeInstanceLock.lock();

    #if !DOUBLE_SIDED_TREES
        glDisable(GL_CULL_FACE);
    #endif
        //render trees trunks
        SCE::ShaderUtils::UseShader(mTreeGlData.trunkShaderProgram);
        SCE::ShaderUtils::BindRootPosition(mTreeGlData.trunkShaderProgram, SCEScene::GetFrameRootPosition());

        SCE::TextureUtils::BindTexture(mTreeGlData.barkTexture, 0, mTreeGlData.barkTexUniform);
        SCE::TextureUtils::BindTexture(mTreeGlData.barkNormalTexture, 1, mTreeGlData.barkNormalTexUniform);

        for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
        {
            SCE::MeshRender::DrawInstances(mTreeGlData.trunkMeshIds[lod], projectionMatrix, viewMatrix);
        }

        //render trees leaves
        SCE::ShaderUtils::UseShader(mTreeGlData.leavesShaderProgram);
        SCE::ShaderUtils::BindRootPosition(mTreeGlData.leavesShaderProgram, SCEScene::GetFrameRootPosition());

        SCE::TextureUtils::BindTexture(mTreeGlData.leafTexture, 0, mTreeGlData.leafTexUniform);
        glUniform1f(mTreeGlData.leavesTranslucencyUniform, TREE_LEAVES_TRANSLUCENCY);

        for(uint lod = 0; lod < TREE_LOD_COUNT; ++lod)
        {
            SCE::MeshRender::DrawInstances(mTreeGlData.leavesMeshIds[lod], projectionMatrix, viewMatrix);
        }

    #if USE_IMPOSTORS

        if(SCE::Quality::Trees::ImpostorShadowEnabled || !isShadowPass)
        {
            //disable cull for impostors to have shadows
            glDisable(GL_CULL_FACE);
            //render tree impostors
            SCE::ShaderUtils::UseShader(mTreeGlData.impostorData.shaderProgram);
            SCE::ShaderUtils::BindRootPosition(mTreeGlData.trunkShaderProgram, SCEScene::GetFrameRootPosition());

            SCE::TextureUtils::BindTexture(mTreeGlData.impostorData.texture, 0,
                                           mTreeGlData.impostorData.textureUniform);
            SCE::TextureUtils::BindTexture(mTreeGlData.impostorData.normalTexture, 1,
                                           mTreeGlData.impostorData.normalUniform);

            SCE::MeshRender::DrawInstances(mTreeGlData.impostorData.meshId, projectionMatrix, viewMatrix);
            glEnable(GL_CULL_FACE);
        }
    #endif

    #if !DOUBLE_SIDED_TREES
        glEnable(GL_CULL_FACE);
    #endif

        mTreeInstanceLock.unlock();
    }
}
