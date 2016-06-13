/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCETerrainTrees.hpp*******/
/**************************************/
#ifndef SCE_SCETERRAINTREES_HPP
#define SCE_SCETERRAINTREES_HPP

#include "SCEDefines.hpp"
#include <vector>
#include <thread>
#include <mutex>

#define TREE_LOD_COUNT 3

namespace SCE
{
    class TerrainTrees
    {
    public :

        TerrainTrees();
        ~TerrainTrees();
        void InitializeTreeLayout(float xOffset, float zOffset,
                                  float startScale, float heightScale, float halfTerrainSize);
        void SpawnTreeInstances(const glm::mat4 &viewMatrix, const glm::mat4 &worldToTerrainspaceMatrix,
                                const glm::vec3 &cameraPosition, float maxDistFromCenter);
        void RenderTrees(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, bool isShadowPass);

    private :

        void UpdateVisibilityAndLOD(glm::mat4 viewMatrix, glm::mat4 worldToTerrainspaceMatrix,
                                    glm::vec3 cameraPosition_scenespace, float maxDistFromCenter, mat4 impostorScaleMat);

        struct ImpostorGLData
        {
            ui16        meshId;
            GLuint      texture;
            GLint       textureUniform;
            GLuint      normalTexture;
            GLint       normalUniform;
            GLuint      shaderProgram;
        };

        struct TreeGLData
        {            
            GLuint  trunkShaderProgram;
            GLuint  leavesShaderProgram;

            GLuint  leafTexture;
            GLuint  barkTexture;
            GLuint  barkNormalTexture;

            GLint   leafTexUniform;
            GLint   barkTexUniform;
            GLint   barkNormalTexUniform;

            GLint   leavesTranslucencyUniform;

            ui16    trunkMeshIds[TREE_LOD_COUNT];
            ui16    leavesMeshIds[TREE_LOD_COUNT];

            ImpostorGLData impostorData;
        };

        struct TreeGroup
        {
            glm::vec2 position;
            float radius;
            float spacing;
        };

        TreeGLData              mTreeGlData;
        std::vector<TreeGroup>  mTreeGroups;
        std::vector<glm::mat4>  mTreeMatrices[TREE_LOD_COUNT];
        std::vector<glm::mat4>  mTreeImpostorMatrices;
        std::vector<glm::vec4>  mTreeImpostorTexMapping;

        std::unique_ptr<std::thread> mUpdateThread;
        std::mutex  mTreeInstanceLock;
        bool        mInstancesUpToDate;
        glm::mat4   mImpostorScaleMat;
    };
}

#endif
