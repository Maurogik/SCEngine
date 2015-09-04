/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCEAssimpLoader.cpp*******/
/**************************************/

#include "../headers/SCEAssimpLoader.hpp"
#include "../headers/SCETools.hpp"

#include <common/vboindexer.hpp>
#include <common/tangentspace.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

using namespace std;


//to use if we do our own indexation
void addNodeMesh(const aiScene* scenePointer,
                 aiNode* node, aiMatrix4x4 parentTransform,
                 bool windCW,
                 std::vector<ushort>&    out_indices,
                 std::vector<vec3>&      out_vertices,
                 std::vector<vec3>&      out_normals,
                 std::vector<vec2>&      out_uvs,
                 std::vector<vec3>&      out_tangents,
                 std::vector<vec3>&      out_bitangents);

//to use if we pre-index meshes with assimp
void addNodeMeshIndexed(const aiScene* scenePointer,
                 aiNode* node, aiMatrix4x4 parentTransform,
                 std::vector<ushort>&    out_indices,
                 std::vector<vec3>&      out_vertices,
                 std::vector<vec3>&      out_normals,
                 std::vector<vec2>&      out_uvs,
                 std::vector<vec3>&      out_tangents,
                 std::vector<vec3>&      out_bitangents);





void SCE::AssimpLoader::LoadModel(const std::string&      path,
                                    std::vector<ushort>&    out_indices,
                                    std::vector<vec3>&      out_vertices,
                                    std::vector<vec3>&      out_normals,
                                    std::vector<vec2>&      out_uvs,
                                    std::vector<vec3>&      out_tangents,
                                    std::vector<vec3>&      out_bitangents,
                                    bool winCW)
{
    Assimp::Importer importer;
    const aiScene* scenePointer = importer.ReadFile(path,
                                                    aiProcess_Triangulate
//                                                    | aiProcess_JoinIdenticalVertices
                                                    | aiProcess_GenSmoothNormals
                                                    | aiProcess_CalcTangentSpace
//                                                    | aiProcess_GenUVCoords
//                                                    | aiProcess_FlipUVs
                                                    | aiProcess_SortByPType
//                                                    | aiProcess_OptimizeGraph
//                                                    | aiProcess_OptimizeMeshes
                                                    | (winCW ? 0 : aiProcess_FlipWindingOrder)
                                                    );

    Debug::Assert(scenePointer, "Could not load model file : " + path + "\n error : " + importer.GetErrorString());

    int meshCount = scenePointer->mNumMeshes;
    Debug::Assert(meshCount > 0, "Model contains 0 meshes !");

    std::vector<ushort>    tmp_indices;
    std::vector<vec3>      tmp_vertices;
    std::vector<vec3>      tmp_normals;
    std::vector<vec2>      tmp_uvs;
    std::vector<vec3>      tmp_tangents;
    std::vector<vec3>      tmp_bitangents;

    addNodeMesh(scenePointer, scenePointer->mRootNode, aiMatrix4x4(), winCW,
                tmp_indices, tmp_vertices, tmp_normals, tmp_uvs, tmp_tangents, tmp_bitangents);


//    computeTangentBasis(tmp_vertices, tmp_uvs, tmp_normals, tmp_tangents, tmp_bitangents);

    //do our own indexing because assimp give weird result on some meshes
    indexVBO_TBN(tmp_vertices, tmp_uvs, tmp_normals, tmp_tangents, tmp_bitangents,
                 out_indices, out_vertices, out_uvs, out_normals, out_tangents, out_bitangents);


}

void addNodeMesh(const aiScene* scenePointer,
                 aiNode* node,
                 aiMatrix4x4 parentTransform,
                 bool windCW,
                 std::vector<ushort>&    out_indices,
                 std::vector<vec3>&      out_vertices,
                 std::vector<vec3>&      out_normals,
                 std::vector<vec2>&      out_uvs,
                 std::vector<vec3>&      out_tangents,
                 std::vector<vec3>&      out_bitangents)
{
    uint meshCount = node->mNumMeshes;

    aiMatrix4x4 transform = node->mTransformation * parentTransform;

    for(uint meshId = 0; meshId < meshCount; ++meshId)
    {
        aiMesh* meshPointer = scenePointer->mMeshes[node->mMeshes[meshId]];

        //assumes non indexed mesh
        for(uint i = 0; i < meshPointer->mNumFaces; ++i)
        {
            SCE::Debug::Assert(meshPointer->mFaces[i].mNumIndices == 3, "Model has non triangle faces !");

            int start = 0, end = 3, step = 1;
            if(windCW)
            {
                start = 2,
                end = -1;
                step = -1;
            }
            for(int f = start; f != end; f += step)
            {
                out_indices.push_back(meshPointer->mFaces[i].mIndices[f]);


                int vert = meshPointer->mFaces[i].mIndices[f];

                aiVector3D aiVert = transform * meshPointer->mVertices[vert];
                aiVector3D aiNorm = transform * meshPointer->mNormals[vert];

                vec3 vertex = vec3(aiVert.x, aiVert.y, aiVert.z);
                vec3 normal = vec3(aiNorm.x, aiNorm.y, aiNorm.z);
                vec2 uv;
                vec3 tangent;
                vec3 bitangent;

                if(meshPointer->HasTangentsAndBitangents())
                {
                    aiVector3D aiTan = transform * meshPointer->mTangents[vert];
                    aiVector3D aiBitan = transform * meshPointer->mBitangents[vert];

                   tangent = vec3(aiTan.x, aiTan.y, aiTan.z);
                   bitangent = vec3(aiBitan.x, aiBitan.y,
                                          aiBitan.z);
                }

                if(meshPointer->HasTextureCoords(0))
                {
                    uv = vec2(meshPointer->mTextureCoords[0][vert].x, meshPointer->mTextureCoords[0][vert].y);
                }

                out_vertices.push_back(vertex);
                out_normals.push_back(normal);
                out_tangents.push_back(tangent);
                out_bitangents.push_back(bitangent);
                out_uvs.push_back(uv);
            }
        }
    }

    for(uint n = 0; n < node->mNumChildren; ++n)
    {
        addNodeMesh(scenePointer, node->mChildren[n], transform, windCW,
                    out_indices, out_vertices, out_normals, out_uvs, out_tangents, out_bitangents);
    }

}


void addNodeMeshIndexed(const aiScene* scenePointer,
                 aiNode* node,
                 aiMatrix4x4 parentTransform,
                 std::vector<ushort>&    out_indices,
                 std::vector<vec3>&      out_vertices,
                 std::vector<vec3>&      out_normals,
                 std::vector<vec2>&      out_uvs,
                 std::vector<vec3>&      out_tangents,
                 std::vector<vec3>&      out_bitangents)
{
    uint meshCount = node->mNumMeshes;

    aiMatrix4x4 transform = node->mTransformation * parentTransform;

    for(uint meshId = 0; meshId < meshCount; ++meshId)
    {
        aiMesh* meshPointer = scenePointer->mMeshes[node->mMeshes[meshId]];

        for(uint i = 0; i < meshPointer->mNumVertices; ++i)
        {
            aiVector3D aiVert = transform * meshPointer->mVertices[i];
            aiVector3D aiNorm = transform * meshPointer->mNormals[i];

            vec3 vertex = vec3(aiVert.x, aiVert.y, aiVert.z);
            vec3 normal = vec3(aiNorm.x, aiNorm.y, aiNorm.z);
            vec2 uv;
            vec3 tangent;
            vec3 bitangent;

            if(meshPointer->HasTangentsAndBitangents())
            {
                aiVector3D aiTan = transform * meshPointer->mTangents[i];
                aiVector3D aiBitan = transform * meshPointer->mBitangents[i];

               tangent = vec3(aiTan.x, aiTan.y, aiTan.z);
               bitangent = vec3(aiBitan.x, aiBitan.y,
                                      aiBitan.z);
            }

            if(meshPointer->HasTextureCoords(0))
            {
                uv = vec2(meshPointer->mTextureCoords[0][i].x, meshPointer->mTextureCoords[0][i].y);
            }

            out_vertices.push_back(vertex);
            out_normals.push_back(normal);
            out_tangents.push_back(tangent);
            out_bitangents.push_back(bitangent);
            out_uvs.push_back(uv);
        }

        for(uint i = 0; i < meshPointer->mNumFaces; ++i)
        {
            SCE::Debug::Assert(meshPointer->mFaces[i].mNumIndices == 3, "Model has non triangle faces !");

            out_indices.push_back(meshPointer->mFaces[i].mIndices[0]);
            out_indices.push_back(meshPointer->mFaces[i].mIndices[1]);
            out_indices.push_back(meshPointer->mFaces[i].mIndices[2]);
        }
    }

    for(uint n = 0; n < node->mNumChildren; ++n)
    {
        addNodeMeshIndexed(scenePointer, node->mChildren[n], transform,
                    out_indices, out_vertices, out_normals, out_uvs, out_tangents, out_bitangents);
    }

}
