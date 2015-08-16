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
    : Component(container, "Mesh::"),
      mIndices(nullptr),
      mVertices(nullptr),
      mNormals(nullptr),
      mUvs(nullptr),
      mTangents(nullptr),
      mBitangents(nullptr)
{
    string fullPath = RESSOURCE_PATH + filename;
    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;

    if(loadOBJ(fullPath.c_str()
               , vertices
               , uvs
               , normals
               , false)) //load with CCW winding order
    {
        vector<ushort> indices;
        vector<vec3>   out_vert;
        vector<vec2>   out_uvs;
        vector<vec3>   out_norm;

        indexVBO(vertices,
                 uvs,
                 normals,
                 indices,
                 out_vert,
                 out_uvs,
                 out_norm);

        initMeshData(&indices, &out_vert, &out_norm, &out_uvs, nullptr, nullptr);
    }
}

SCE::Mesh::Mesh(SCEHandle<Container> &container
                , vector<ushort> *indices
                , vector<vec3> *vertices
                , vector<vec3> *normals
                , vector<vec2> *uvs
                , vector<vec3> *tangents
                , vector<vec3> *bitangents)
    : Component(container, "Mesh::"),
      mIndices(nullptr),
      mVertices(nullptr),
      mNormals(nullptr),
      mUvs(nullptr),
      mTangents(nullptr),
      mBitangents(nullptr)
{
    initMeshData(indices, vertices, normals, uvs, tangents, bitangents);
}

void SCE::Mesh::initMeshData(  vector<ushort> *indices,
                               vector<vec3> *vertices,
                               vector<vec3> *normals,
                               vector<vec2> *uvs,
                               vector<vec3> *tangents,
                               vector<vec3> *bitangents)
{
    if(indices)
    {
        mIndices    = new vector<ushort>(*indices);
    }
    if(vertices)
    {
        mVertices   = new vector<vec3>(*vertices);
    }
    if(normals)
    {
        mNormals    = new vector<vec3>(*normals);
    }
    if(uvs)
    {
        mUvs        = new vector<vec2>(*uvs);
    }
    if(tangents)
    {
        mTangents   = new vector<vec3>(*tangents);
    }
    if(bitangents)
    {
        mBitangents = new vector<vec3>(*bitangents);
    }
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
SCEHandle<Mesh> SCE::Mesh::AddSphereMesh(SCEHandle<Container> &container, float radius, float tesselation)
{
    float fTess     = (float)tesselation;
    float angleStep = 90.0f / glm::pow(2.0f, fTess);
    int nbSteps     = 360.0f / angleStep;

    //indicies of vertices, normal and uvs stored by angle over x, angle over y;
    short angleIndices[nbSteps][nbSteps];
    //set the array to -1 as default value
    for(int i = 0; i < nbSteps; ++i)
    {
        for(int j = 0; j < nbSteps; ++j)
        {
            angleIndices[i][j] = -1;
        }
    }

    vector<vec3>    vertices;
    vector<vec3>    normals;
    vector<vec2>    uvs;
    vector<ushort>  indices;

    //loop over all the angles to make a full 360 over x and y axis
    for(int xStep = 0; xStep < nbSteps / 2; ++xStep)
    {
        float xAngle = xStep * angleStep;

        glm::quat xRot[2] = {
            glm::angleAxis(xAngle, 1.0f, 0.0f, 0.0f),
            glm::angleAxis(xAngle + angleStep, 1.0f, 0.0f, 0.0f)
        };

        for(int yStep = 0; yStep < nbSteps; ++yStep)
        {
            float yAngle = yStep * angleStep;

            glm::quat yRot[2] = {
                glm::angleAxis(yAngle, 0.0f, 1.0f, 0.0f),
                glm::angleAxis(yAngle + angleStep, 0.0f, 1.0f, 0.0f)
            };

            float u[2] = {
                float(xStep)/float(nbSteps / 2),
                float(xStep+1)/float(nbSteps / 2)
            };

            float v[2] = {
                float(yStep)/float(nbSteps),
                float(yStep+1)/float(nbSteps)
            };

            ushort vertIndices[4];
            int indCount = 0;
            //loop over the four needed vertices to construct a quad (2 tris) in the order x1y1, x1y2, x2y1, x2y2
            for(int stepAddX = 0; stepAddX <= 1; ++stepAddX)
            {
                for(int stepAddY = 0; stepAddY <= 1; ++stepAddY)
                {
                    vec3 dir = yRot[stepAddY] * xRot[stepAddX] * vec3(0.0f, 1.0f, 0.0f);
                    vec3 normalizedDir = normalize(dir);
                    dir = normalizedDir;
                    dir *= radius;

                    if(angleIndices[xStep + stepAddX][yStep + stepAddY] < 0)
                    { //first encouter of this vertice
                        vertices.push_back(dir);
                        normals.push_back(normalizedDir);
                        uvs.push_back(vec2(u[stepAddX], v[stepAddY]));//push dummy uv for now, fix later
                        angleIndices[xStep + stepAddX][yStep + stepAddY] = vertices.size() - 1;
                    }
                    vertIndices[indCount] = angleIndices[xStep + stepAddX][yStep + stepAddY];
                    ++indCount;
                }
            }
            //push the two triangles
            indices.push_back(vertIndices[0]);
            indices.push_back(vertIndices[1]);
            indices.push_back(vertIndices[2]);

            indices.push_back(vertIndices[3]);
            indices.push_back(vertIndices[2]);
            indices.push_back(vertIndices[1]);
        }
    }

    return container->AddComponent<Mesh>(&indices,
                                         &vertices,
                                         &normals,
                                         &uvs,
                                         nullptr,
                                         nullptr);
}

SCEHandle<Mesh> SCE::Mesh::AddCubeMesh(SCEHandle<Container> &container, float cubeSize)
{
    float size = cubeSize / 2.0f;
    vector<vec3> vertices = vector<vec3>
    {
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

    vector<vec2> uvs = vector<vec2>
    {
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

    vec3 normalArray[8]
    {
        vec3(0.0f, 0.0f, 1.0f), //front
        vec3(0.0f, 0.0f, -1.0f), //back
        vec3(0.0f, 1.0f,  0.0f), //top
        vec3(0.0f, -1.0f, 0.0f), //bottom
        vec3( 1.0f, 0.0f, 0.0f), //right
        vec3(-1.0f, 0.0f, 0.0f), //left
    };

    vector<vec3> normals;
    for(int i = 0; i < 6; ++i)
    {
        vec3 norm = normalArray[i];
        for(int j = 0; j < 4; ++j)
        {
            normals.push_back(norm);
        }
    }

    vector<ushort> indices = vector<ushort>
    {
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23    // left
    };

    return container->AddComponent<Mesh>(&indices, &vertices, &normals, &uvs, (std::vector<vec3> *)nullptr, (std::vector<vec3> *)nullptr);
}

SCEHandle<Mesh> Mesh::AddQuadMesh(SCEHandle<Container> &container, float width, float height)
{
    vector<vec3> vertices = vector<vec3>
    {
        vec3(-width/2.0f, -height/2.0f,  0.0f),
        vec3( width/2.0f, -height/2.0f,  0.0f),
        vec3( width/2.0f,  height/2.0f,  0.0f),
        vec3(-width/2.0f,  height/2.0f,  0.0f)
    };

    vector<vec2> uvs = vector<vec2>{
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f)
    };

    vector<vec3> normals;
    normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    vector<ushort> indices = vector<ushort>
    {
        2,  1,  0,      0,  3,  2
    };

    return container->AddComponent<Mesh>(&indices, &vertices, &normals, &uvs, (std::vector<vec3> *)nullptr, (std::vector<vec3> *)nullptr);
}

SCEHandle<Mesh> Mesh::AddConeMesh(SCEHandle<Container> &container, float length, float angle, float tesselation)
{
    float fTess         = tesselation;
    float angleStep     = 90.0f / glm::pow(2.0f, fTess);
    float lengthStep    = length / glm::pow(2.0f, fTess);
    int nbAngleSteps    = 360.0f / angleStep;
    int nbLengthSteps   = length / lengthStep;

    //indicies of vertices, normal and uvs stored by angle over x, angle over y;
    short viewedVertices[nbAngleSteps][nbAngleSteps];
    //set the array to -1 as default value
    for(int i = 0; i < nbAngleSteps; ++i)
    {
        for(int j = 0; j < nbAngleSteps; ++j)
        {
            viewedVertices[i][j] = -1;
        }
    }

    vector<vec3>    vertices;
    vector<vec3>    normals;
    vector<vec2>    uvs;
    vector<ushort>  indices;

    glm::quat coneRotation = glm::angleAxis(angle, 1.0f, 0.0f, 0.0f);

    //loop over all the angles to make a full 360 over x and y axis
    for(int zAngleStep = 0; zAngleStep < nbAngleSteps - 1; ++zAngleStep)
    {
        float zAngle = zAngleStep * angleStep;

        glm::quat zRot[2] = {
            glm::angleAxis(zAngle, 0.0f, 0.0f, 1.0f) * coneRotation,
            glm::angleAxis(zAngle + angleStep, 0.0f, 0.0f, 1.0f) * coneRotation
        };

        for(int zPosStep = 0; zPosStep < nbLengthSteps - 1; ++zPosStep)
        {
            float zPos[2] = {
                zPosStep * lengthStep,
                (zPosStep + 1) * lengthStep
            };

            ushort vertIndices[4];
            int indCount = 0;
            //loop over the four needed vertices to construct a quad (2 tris) in the order x1y1, x1y2, x2y1, x2y2
            for(int subStepZAngle = 0; subStepZAngle <= 1; ++subStepZAngle)
            {
                for(int subStepZPos = 0; subStepZPos <= 1; ++subStepZPos)
                {

                    vec3 pos = zRot[subStepZAngle] * vec3(0.0f, 0.0f, zPos[subStepZPos]);

                    if(viewedVertices[zAngleStep + subStepZAngle][zPosStep + subStepZPos] < 0)
                    { //first encouter of this vertice
                        vertices.push_back(pos);///////////////////////TODO COMPUTE NORMALS
                        vec3 normal = normalize(pos - vec3(0.0f, 0.0f, zPos[subStepZPos]));
                        normals.push_back(normal);
                        uvs.push_back(vec2(0.0f, 0.0f));//push dummy uv for now, fix later
                        viewedVertices[zAngleStep + subStepZAngle][zPosStep + subStepZPos] = vertices.size() - 1;
                    }
                    vertIndices[indCount] = viewedVertices[zAngleStep + subStepZAngle][zPosStep + subStepZPos];
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
    return container->AddComponent<Mesh>(&indices, &vertices, &normals, &uvs, (vector<vec3>*) nullptr, (vector<vec3>*) nullptr);
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

    mIndices    = nullptr;
    mVertices   = nullptr;
    mNormals    = nullptr;
    mUvs        = nullptr;
    mTangents   = nullptr;
    mBitangents = nullptr;
}

