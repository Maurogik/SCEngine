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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/constants.hpp>

#include <sstream>

using namespace SCE;
using namespace std;

SCE::Mesh::Mesh(SCEHandle<Container> &container, const string& filename)
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

SCE::Mesh::Mesh(SCEHandle<Container> &container
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

SCEHandle<Mesh> SCE::Mesh::AddCustomMesh(SCEHandle<Container> &container, const string &filename)
{
    return container->AddComponent<Mesh>(filename);
}

//Tesselation : number of time the basic (90°) angle is divided by 2 to get the angle step, range from 0 to infinity
SCEHandle<Mesh> SCE::Mesh::AddSphereMesh(SCEHandle<Container> &container, const ushort &radius, const ushort &tesselation)
{
    float fTess = (float)tesselation;
    float angleStep = 90.0f / glm::pow(2.0f, fTess);
    int nbSteps = 360.0f / angleStep;

    //indicies of vertices, normal and uvs stored by angle over x, angle over y;
    short angleIndices[nbSteps][nbSteps];
    //set the array to -1 as default value
    for(int i = 0; i < nbSteps; ++i){
        for(int j = 0; j < nbSteps; ++j){
            angleIndices[i][j] = -1;
        }
    }

    vector<vec3> vertices;
    vector<vec3> normals;
    vector<vec2> uvs;
    vector<ushort> indices;

    //loop over all the angles to make a full 360 over x and y axis
    for(int xStep = 0; xStep < nbSteps / 2; ++xStep){
        float xAngle = xStep * angleStep;

        glm::quat xRot[2] = {
            glm::angleAxis(xAngle, 1.0f, 0.0f, 0.0f),
            glm::angleAxis(xAngle + angleStep, 1.0f, 0.0f, 0.0f)
        };

        for(int yStep = 0; yStep < nbSteps; ++yStep){
            float yAngle = yStep * angleStep;

            if(yStep == nbSteps - 1){

                SCE::Debug::PrintMessage("stop !");
            }

            glm::quat yRot[2] = {
                glm::angleAxis(yAngle, 0.0f, 1.0f, 0.0f),
                glm::angleAxis(yAngle + angleStep, 0.0f, 1.0f, 0.0f)
            };

            ushort vertIndices[4];
            int indCount = 0;
            //loop over the four needed vertices to construct a quad (2 tris) in the order x1y1, x1y2, x2y1, x2y2
            for(int stepAddX = 0; stepAddX <= 1; ++stepAddX){
                for(int stepAddY = 0; stepAddY <= 1; ++stepAddY){

                    vec3 dir = yRot[stepAddY] * xRot[stepAddX] * vec3(0.0f, 1.0f, 0.0f);
                    vec3 normalizedDir = normalize(dir);
                    dir = normalizedDir;
                    dir *= radius;

                    if(angleIndices[xStep + stepAddX][yStep + stepAddY] < 0) { //first encouter of this vertice
                        vertices.push_back(dir);
                        normals.push_back(normalizedDir);
                        uvs.push_back(vec2(0.0f, 0.0f));//push dummy uv for now, fix later
                        angleIndices[xStep + stepAddX][yStep + stepAddY] = vertices.size() - 1;
                    }
                    vertIndices[indCount] = angleIndices[xStep + stepAddX][yStep + stepAddY];
                    ++indCount;
                }
            }
            //push the two triangles
            indices.push_back(vertIndices[2]);
            indices.push_back(vertIndices[1]);
            indices.push_back(vertIndices[0]);

            indices.push_back(vertIndices[1]);
            indices.push_back(vertIndices[2]);
            indices.push_back(vertIndices[3]);
        }
    }

    return container->AddComponent<Mesh>(&indices, &vertices, &normals, &uvs, (vector<vec3>*) 0l, (vector<vec3>*) 0l);
}

SCEHandle<Mesh> SCE::Mesh::AddCubeMesh(SCEHandle<Container> &container, const ushort &size)
{

    vector<vec3> vertices = vector<vec3>{
        // Front face
        vec3(-size, -size,  size),
        vec3( size, -size,  size),
        vec3( size,  size,  size),
        vec3(-size,  size,  size),

        // Back face
        vec3(-size, -size, -size),
        vec3(-size,  size, -size),
        vec3( size,  size, -size),
        vec3( size, -size, -size),

        // Top face
        vec3(-size,  size, -size),
        vec3(-size,  size,  size),
        vec3( size,  size,  size),
        vec3( size,  size, -size),

        // Bottom face
        vec3(-size, -size, -size),
        vec3( size, -size, -size),
        vec3( size, -size,  size),
        vec3(-size, -size,  size),

        // Right face
        vec3( size, -size, -size),
        vec3( size,  size, -size),
        vec3( size,  size,  size),
        vec3( size, -size,  size),

        // Left face
        vec3(-size, -size, -size),
        vec3(-size, -size,  size),
        vec3(-size,  size,  size),
        vec3(-size,  size, -size)
    };

    vector<vec2> uvs = vector<vec2>{
           // Front face
           vec2(0.33f, 0.25f),
           vec2(0.66f, 0.25f),
           vec2(0.66f, 0.00f),
           vec2(0.33f, 0.00f),
           // Back face
           vec2(0.33f, 0.50f),
           vec2(0.33f, 0.75f),
           vec2(0.66f, 0.75f),
           vec2(0.66f, 0.50f),
           // Top face
           vec2(0.33f, 1.00f),
           vec2(0.33f, 0.75f),
           vec2(0.66f, 1.00f),
           vec2(0.66f, 0.75f),
           // Bottom face
           vec2(0.33f, 0.25f),
           vec2(0.66f, 0.25f),
           vec2(0.66f, 0.50f),
           vec2(0.33f, 0.50f),
           // Right face
           vec2(0.66f, 0.25f),
           vec2(1.00f, 0.25f),
           vec2(1.00f, 0.50f),
           vec2(0.66f, 0.50f),
           // Left face
           vec2(0.33f, 0.25f),
           vec2(0.33f, 0.50f),
           vec2(0.00f, 0.50f),
           vec2(0.00f, 0.25f)
    };

    vec3 normalArray[8]{
        vec3(0.0f, 0.0f, 1.0f), //front
        vec3(0.0f, 0.0f, -1.0f), //back
        vec3(0.0f, 1.0f,  0.0f), //top
        vec3(0.0f, -1.0f, 0.0f), //bottom
        vec3( 1.0f, 0.0f, 0.0f), //right
        vec3(-1.0f, 0.0f, 0.0f), //left
    };

    vector<vec3> normals = vector<vec3>();
    for(int i = 0; i < 6; ++i){
        vec3 norm = normalArray[i];
        for(int j = 0; j < 4; ++j){
            normals.push_back(norm);
        }
    }

    vector<ushort> indices = vector<ushort>{
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23    // left
    };

    return container->AddComponent<Mesh>(&indices, &vertices, &normals, &uvs, (std::vector<vec3> *)0l, (std::vector<vec3> *)0l);
}

SCEHandle<Mesh> Mesh::AddCustomMesh(SCEHandle<Container> &container, std::vector<ushort> *indices, std::vector<vec3> *vertices
                                 , std::vector<vec3> *normals, std::vector<vec2> *uvs, std::vector<vec3> *tangents, std::vector<vec3> *bitangents)
{
    return container->AddComponent<Mesh>(indices, vertices, normals, uvs, tangents, bitangents);
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

