/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshLoader.cpp********/
/**************************************/


#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"

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



SCEMeshLoader* SCEMeshLoader::s_instance = nullptr;


SCEMeshLoader::SCEMeshLoader()
    : mMeshIds(),
      mMeshData(),
      mNextId(0)
{

}

void SCEMeshLoader::Init()
{
    Debug::Assert(!s_instance, "An instance of the Mesh loader system already exists");
    s_instance = new SCEMeshLoader();
}

void SCEMeshLoader::CleanUp()
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");
    delete s_instance;
}

uint SCEMeshLoader::CreateMeshFromFile(const string& meshFileName)
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");

    if(s_instance->mMeshIds.count(meshFileName) > 0)
    {
        return s_instance->mMeshIds[meshFileName];
    }

    string fullPath = RESSOURCE_PATH + meshFileName;
    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;

    uint id = -1;

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

        vector<vec3> tangent(0);
        vector<vec3> bitangent(0);
        id = s_instance->addMeshData(meshFileName, indices, out_vert, out_norm, out_uvs, tangent, bitangent);
    }

    return id;
}

uint SCEMeshLoader::CreateSphereMesh(float tesselation)
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");

    string meshName = "Sphere" + std::to_string(tesselation);
    if(s_instance->mMeshIds.count(meshName) > 0)
    {
        return s_instance->mMeshIds[meshName];
    }

    //Tesselation : number of time the basic (90°) angle is divided by 2 to get the angle step,
    //range from 0 to infinity

    float radius    = 1.0f;
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
            //loop over the four needed vertices to construct a quad (2 tris)
            //in the order x1y1, x1y2, x2y1, x2y2
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
                        uvs.push_back(vec2(u[stepAddX], v[stepAddY]));
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

    vector<vec3> tangent(0);
    vector<vec3> bitangent(0);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangent, bitangent);
}

uint SCEMeshLoader::CreateConeMesh(float angle, float tesselation)
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");

    string meshName = "Cone" + std::to_string(angle) + "_" + std::to_string(tesselation);
    if(s_instance->mMeshIds.count(meshName) > 0)
    {
        return s_instance->mMeshIds[meshName];
    }

    float length        = 1.0f;
    float fTess         = tesselation;
    //atefacts appear if angleStep is to low so clamp to min value
    float angleStep     = glm::max(90.0f / glm::pow(2.0f, fTess), 5.0f);
    float lengthStep    = glm::max(length / glm::pow(2.0f, fTess), 0.2f);
    int nbAngleSteps    = 360.0f / angleStep;
    int nbLengthSteps   = length / lengthStep;

    //indicies of vertices, normal and uvs stored by [angle over x, distance over z]
    short viewedVertices[nbAngleSteps + 1][nbLengthSteps + 1];
    //set the array to -1 as default value
    for(int i = 0; i < nbAngleSteps + 1; ++i)
    {
        for(int j = 0; j < nbLengthSteps + 1; ++j)
        {
            viewedVertices[i][j] = -1;
        }
    }

    vector<vec3>    vertices;
    vector<vec3>    normals;
    vector<vec2>    uvs;
    vector<ushort>  indices;

    //Add the vertex at the center of the cone's end
    vertices.push_back(vec3(0.0, 0.0, float(nbLengthSteps) * lengthStep));
    normals.push_back(vec3(0.0, 0.0, 1.0));
    uvs.push_back(vec2(0.0, 0.0));
    short endVertexIndex = 0;

    glm::quat coneRotation = glm::angleAxis(angle, 1.0f, 0.0f, 0.0f);

    //Generate vertices for cone surface
    //loop over all the angles to make a full 360 around the cone's Z axis
    for(int zAngleStep = 0; zAngleStep < nbAngleSteps; ++zAngleStep)
    {
        float zAngle = zAngleStep * angleStep;

        glm::quat zRot[2] =
        {
            glm::angleAxis(zAngle, 0.0f, 0.0f, 1.0f) * coneRotation,
            glm::angleAxis(zAngle + angleStep, 0.0f, 0.0f, 1.0f) * coneRotation
        };

        for(int zPosStep = 0; zPosStep < nbLengthSteps; ++zPosStep)
        {
            float zPos[2] =
            {
                float(zPosStep) * lengthStep,
                float(zPosStep + 1) * lengthStep
            };

            //Compute uvs
            float u[2] = {
                float(zAngleStep) / float(nbAngleSteps),
                float(zAngleStep + 1) / float(nbAngleSteps)
            };

            float v[2] = {
                float(zPosStep) / float(nbLengthSteps),
                float(zPosStep + 1) / float(nbLengthSteps)
            };

            ushort vertIndices[4];
            int indCount = 0;
            //loop over the four needed vertices to construct a quad (2 tris)
            //in the order x1y1, x1y2, x2y1, x2y2
            for(int subStepZAngle = 0; subStepZAngle <= 1; ++subStepZAngle)
            {
                for(int subStepZPos = 0; subStepZPos <= 1; ++subStepZPos)
                {
                    vec3 pos = zRot[subStepZAngle] * vec3(0.0f, 0.0f, zPos[subStepZPos]);
                    //correct z pos so the surface is at the same z;
                    pos.z = zPos[subStepZPos];

                    if(viewedVertices[zAngleStep + subStepZAngle][zPosStep + subStepZPos] < 0)
                    { //first encouter of this vertex
                        vertices.push_back(pos);
                        //Normal(approx) is dir from pos on z axis to pos on the surface of the cone
                        //add small offset to handle the case of the vertex at tip of the cone
                        vec3 normal = normalize(pos - vec3(0.0f, 0.0f, zPos[subStepZPos] + 0.001f));
                        normals.push_back(normal);
                        uvs.push_back(vec2(u[subStepZAngle], v[subStepZPos]));
                        viewedVertices[zAngleStep + subStepZAngle][zPosStep + subStepZPos] = vertices.size() - 1;
                    }
                    vertIndices[indCount] = viewedVertices[zAngleStep + subStepZAngle][zPosStep + subStepZPos];
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

            //Generate vertices for cone end
            if(zPosStep == nbLengthSteps - 1)
            {
                //push a triangle at the end of the cone
                indices.push_back(vertIndices[3]);
                indices.push_back(vertIndices[1]);
                indices.push_back(endVertexIndex);
            }
        }
    }

    vector<vec3> tangent(0);
    vector<vec3> bitangent(0);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangent, bitangent);
}

uint SCEMeshLoader::CreateQuadMesh()
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");

    float width = 2.0f;
    float height = 2.0f;
    string meshName = "Quad";
    if(s_instance->mMeshIds.count(meshName) > 0)
    {
        return s_instance->mMeshIds[meshName];
    }

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
    normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    normals.push_back(vec3(0.0f, 0.0f, 1.0f));


    vector<ushort> indices = vector<ushort>
    {
        2,  1,  0,      0,  3,  2
    };

    vector<vec3> tangent(0);
    vector<vec3> bitangent(0);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangent, bitangent);
}

uint SCEMeshLoader::CreateCubeMesh()
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");

    string meshName = "Cube";
    if(s_instance->mMeshIds.count(meshName) > 0)
    {
        return s_instance->mMeshIds[meshName];
    }

    float halfSize = 0.5f;
    vector<vec3> vertices = vector<vec3>
    {
        // Front face
        vec3(-halfSize, -halfSize,  halfSize),
        vec3( halfSize, -halfSize,  halfSize),
        vec3( halfSize,  halfSize,  halfSize),
        vec3(-halfSize,  halfSize,  halfSize),

        // Back face
        vec3(-halfSize, -halfSize,  -halfSize),
        vec3( halfSize, -halfSize,  -halfSize),
        vec3( halfSize,  halfSize,  -halfSize),
        vec3(-halfSize,  halfSize,  -halfSize),

        // Top face
        vec3(-halfSize,  halfSize, -halfSize),
        vec3(-halfSize,  halfSize,  halfSize),
        vec3( halfSize,  halfSize,  halfSize),
        vec3( halfSize,  halfSize, -halfSize),

        // Bottom face
        vec3(-halfSize,  -halfSize, -halfSize),
        vec3(-halfSize,  -halfSize,  halfSize),
        vec3( halfSize,  -halfSize,  halfSize),
        vec3( halfSize,  -halfSize, -halfSize),

        // Right face
        vec3( halfSize, -halfSize, -halfSize),
        vec3( halfSize,  halfSize, -halfSize),
        vec3( halfSize,  halfSize,  halfSize),
        vec3( halfSize, -halfSize,  halfSize),

        // Left face
        vec3(-halfSize, -halfSize, -halfSize),
        vec3(-halfSize,  halfSize, -halfSize),
        vec3(-halfSize,  halfSize,  halfSize),
        vec3(-halfSize, -halfSize,  halfSize),
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
        2,  1,  0,      3,  2,  0,    // front
        4,  5,  6,      4,  6,  7,    // back
        10,  8,  11,    8,  10, 9,    // top
        15, 12, 14,     13, 14, 12,   // bottom
        16, 19, 17,     19, 18, 17,   // right
        21, 23, 20,     21, 22, 23    // left
    };

    vector<vec3> tangent(0);
    vector<vec3> bitangent(0);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangent, bitangent);
}

uint SCEMeshLoader::CreateCustomMesh(const std::vector<ushort>& indices,
                                     const std::vector<vec3>& vertices,
                                     const std::vector<vec3>& normals,
                                     const std::vector<vec2>& uvs,
                                     const std::vector<vec3>& tangents,
                                     const std::vector<vec3>& bitangents)
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");
    string meshName = "Custom" + std::to_string(s_instance->mNextId);
    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangents, bitangents);
}

void SCEMeshLoader::DeleteMesh(uint meshId)
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");

    auto it = find_if(begin(s_instance->mMeshIds),
                      end  (s_instance->mMeshIds),
                      [&meshId](std::pair<string, uint> entry)
    { return entry.second == meshId; } );

    if(it != end(s_instance->mMeshIds))
    {
        Internal::Log("Delete mesh : " + it->first);
        s_instance->mMeshIds.erase(it);
    }
}

const MeshData&SCEMeshLoader::GetMeshData(uint meshId)
{
    Debug::Assert(s_instance, "No Mesh loader system instance found, Init the system before using it");
    Debug::Assert(s_instance->mMeshData.count(meshId) > 0, "No mesh found for id : " + to_string(meshId));

    return s_instance->mMeshData[meshId];
}

uint SCEMeshLoader::addMeshData(const string& meshName,
                                const std::vector<ushort>& indices,
                                const std::vector<vec3>& vertices,
                                const std::vector<vec3>& normals,
                                const std::vector<vec2>& uvs,
                                const std::vector<vec3>& tangents,
                                const std::vector<vec3>& bitangents)
{
    uint id = mNextId++;
    mMeshIds[meshName] = id;
    mMeshData[id].indices = indices;
    mMeshData[id].vertices = vertices;
    mMeshData[id].normals = normals;
    mMeshData[id].uvs = uvs;
    mMeshData[id].tangents = tangents;
    mMeshData[id].bitangents = bitangents;

    return id;
}
