/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/*********** FILE : Mesh.cpp **********/
/**************************************/

#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/Container.hpp"
#include "../headers/Mesh.hpp"
#include "../headers/SCEMeshLoader.hpp"

#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/tangentspace.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/constants.hpp>

#include <sstream>

using namespace SCE;
using namespace std;

SCE::Mesh::Mesh(SCEHandle<Container> &container, const string& filename, bool windCW)
    : Component(container, "Mesh::")
{
    mMeshId = SCEMeshLoader::CreateMeshFromFile(filename, windCW);
}

SCE::Mesh::Mesh(SCEHandle<Container> &container
                , const std::vector<ushort>& indices
                , const std::vector<vec3>& vertices
                , const std::vector<vec3>& normals
                , const std::vector<vec2>& uvs
                , const std::vector<vec3>& tangents
                , const std::vector<vec3>& bitangents)
    : Component(container, "Mesh::")
{
    mMeshId = SCEMeshLoader::CreateCustomMesh(indices, vertices, normals, uvs, tangents, bitangents);
}

SCE::Mesh::Mesh(SCEHandle<Container> &container, uint meshId)
    : Component(container, "Mesh::")
{
    mMeshId = meshId;
}

const MeshData&Mesh::GetMeshData() const
{
    return SCEMeshLoader::GetMeshData(mMeshId);
}

uint Mesh::GetMeshId()
{
    return mMeshId;
}

SCEHandle<Mesh> SCE::Mesh::AddCustomMesh(SCEHandle<Container> &container, const string &filename)
{
    return container->AddComponent<Mesh>(filename);
}

SCEHandle<Mesh> SCE::Mesh::AddSphereMesh(SCEHandle<Container> &container, float tesselation)
{
    uint meshId = SCEMeshLoader::CreateSphereMesh(tesselation);
    return container->AddComponent<Mesh>(meshId);
}

SCEHandle<Mesh> SCE::Mesh::AddCubeMesh(SCEHandle<Container> &container)
{
    uint meshId = SCEMeshLoader::CreateCubeMesh();
    return container->AddComponent<Mesh>(meshId);
}

SCEHandle<Mesh> Mesh::AddQuadMesh(SCEHandle<Container> &container)
{
    uint meshId = SCEMeshLoader::CreateQuadMesh();
    return container->AddComponent<Mesh>(meshId);
}

SCEHandle<Mesh> Mesh::AddConeMesh(SCEHandle<Container> &container,
                                  float angle,
                                  float tesselation)
{
    uint meshId = SCEMeshLoader::CreateConeMesh(angle, tesselation);
    return container->AddComponent<Mesh>(meshId);
}

SCEHandle<Mesh> Mesh::AddCustomMesh(SCEHandle<Container> &container, const std::vector<ushort>& indices, const std::vector<vec3>& vertices
                                 , const std::vector<vec3>& normals, const std::vector<vec2>& uvs, const std::vector<vec3>& tangents, const std::vector<vec3>& bitangents)
{
    return container->AddComponent<Mesh>(indices, vertices, normals, uvs, tangents, bitangents);
}


