/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/*********** FILE : Mesh.cpp **********/
/**************************************/

#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/Container.hpp"
#include "../headers/Mesh.hpp"
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/tangentspace.hpp>

using namespace SCE;
using namespace std;

SCE::Mesh::Mesh(SCE::Container &container)
    : Component(container, "Mesh::")
    , mIndices(0l), mVertices(0l), mNormals(0l), mUvs(0l), mTangents(0l), mBitangents(0l)
{
}

SCE::Mesh::Mesh(SCE::Container &container, const string& filename)
    : Component(container, "Mesh::")
    , mIndices(0l), mVertices(0l), mNormals(0l), mUvs(0l), mTangents(0l), mBitangents(0l)
{
    string fullPath = RESSOURCE_PATH + filename;
    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
//    vector<vec3> tangents;
//    vector<vec3> bitangents;

    if(loadOBJ(fullPath.c_str()
               , vertices
               , uvs
               , normals))
    {
        //computeTangentBasis(vertices, uvs, normals, tangents, bitangents);

        vector<ushort> indices;
        vector<vec3>   out_vert;
        vector<vec2>   out_uvs;
        vector<vec3>   out_norm;
//        vector<vec3>   out_tan;
//        vector<vec3>   out_bitan;

        indexVBO(vertices
                 , uvs
                 , normals
                 , indices
                 , out_vert
                 , out_uvs
                 , out_norm);

        initMeshData(&indices, &out_vert, &out_norm, &out_uvs, 0l, 0l);
    }
}

SCE::Mesh::Mesh(SCE::Container &container, const ushort &radius, const ushort &tesselation)
    : Component(container, "Mesh::")
    , mIndices(0l), mVertices(0l), mNormals(0l), mUvs(0l), mTangents(0l), mBitangents(0l)
{

}

SCE::Mesh::Mesh(SCE::Container &container, const ushort &width, const ushort &height, const ushort &depth)
    : Component(container, "Mesh::")
    , mIndices(0l), mVertices(0l), mNormals(0l), mUvs(0l), mTangents(0l), mBitangents(0l)
{

}

SCE::Mesh::Mesh(Container &container
                , vector<ushort> *indices
                , vector<vec3> *vertices
                , vector<vec3> *normals
                , vector<vec2> *uvs
                , vector<vec3> *tangents
                , vector<vec3> *bitangents)
    : Component(container, "Mesh::")
    , mIndices(0l), mVertices(0l), mNormals(0l), mUvs(0l), mTangents(0l), mBitangents(0l)
{
    initMeshData(indices, vertices, normals, uvs, tangents, bitangents);
}

void SCE::Mesh::initMeshData(  vector<ushort> *indices
                             , vector<vec3> *vertices
                             , vector<vec3> *normals
                             , vector<vec2> *uvs
                             , vector<vec3> *tangents
                             , vector<vec3> *bitangents)
{
    if(indices)
        mIndices    = new vector<ushort>(*indices);
    if(vertices)
        mVertices   = new vector<vec3>(*vertices);
    if(normals)
        mNormals    = new vector<vec3>(*normals);
    if(uvs)
        mUvs        = new vector<vec2>(*uvs);
    if(tangents)
        mTangents   = new vector<vec3>(*tangents);
    if(bitangents)
        mBitangents = new vector<vec3>(*bitangents);
}

SCE::Mesh::~Mesh()
{
    resetMeshData();
}

std::vector<ushort> *SCE::Mesh::GetIndices()
{
    return mIndices;
}

std::vector<vec3> *SCE::Mesh::GetVertices()
{
    return mVertices;
}

std::vector<vec3> *SCE::Mesh::GetNormals()
{
    return mNormals;
}

std::vector<vec2> *SCE::Mesh::GetUvs()
{
    return mUvs;
}

std::vector<vec3> *SCE::Mesh::GetTangents()
{
    return mTangents;
}

std::vector<vec3> *SCE::Mesh::GetBitangents()
{
    return mBitangents;
}

SCE::Mesh &SCE::Mesh::AddCustomMesh(SCE::Container& container, const string &filename)
{
    return container.AddComponent<Mesh>();
}

SCE::Mesh &SCE::Mesh::AddSphereMesh(SCE::Container &container, const ushort &radius, const ushort &tesselation)
{
    return container.AddComponent<Mesh>();
}

SCE::Mesh &SCE::Mesh::AddCubeMesh(SCE::Container &container, const ushort &width, const ushort &height, const ushort &depth)
{
    return container.AddComponent<Mesh>();
}

void SCE::Mesh::resetMeshData()
{
    SECURE_DELETE(mIndices);
    SECURE_DELETE(mVertices);
    SECURE_DELETE(mUvs);
    SECURE_DELETE(mNormals);
    SECURE_DELETE(mTangents);
    SECURE_DELETE(mBitangents);

    mIndices    = 0l;
    mVertices   = 0l;
    mNormals    = 0l;
    mUvs        = 0l;
    mTangents   = 0l;
    mBitangents = 0l;
}

