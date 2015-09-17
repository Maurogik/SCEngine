/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshLoader.cpp********/
/**************************************/


#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/constants.hpp>

#include <fstream>
#include <sstream>
#include <algorithm>

using namespace SCE;
using namespace std;


//File scope functions
namespace
{
    #define SEP ';'

    template<typename T>
    T readLine(string& line)
    {
        return T();
    }

    template<>
    ushort readLine(string &line)
    {
        Debug::Assert(std::count(begin(line), end(line), SEP) == 0, "Can't parse line");

        return std::stoi(line);
    }

    template<>
    glm::vec2 readLine(string &line)
    {
        Debug::Assert(std::count(begin(line), end(line), SEP) == 1, "Can't parse line");

        int sepPos = line.find_first_of(SEP);
        string strX = line.substr(0, sepPos);
        string strY = line.substr(sepPos + 1);
        vec2 res;
        res.x = stof(strX);
        res.y = stof(strY);

        return res;
    }

    template<>
    glm::vec3 readLine(string &line)
    {
        Debug::Assert(std::count(begin(line), end(line), SEP) == 2, "Can't parse line");
        int sepPos = line.find_first_of(SEP);
        int sepPos2 = line.find_last_of(SEP);
        string strX = line.substr(0, sepPos);
        string strY = line.substr(sepPos + 1, sepPos2 - sepPos - 1);
        string strZ = line.substr(sepPos2 + 1);
        vec3 res;
        res.x = stof(strX);
        res.y = stof(strY);
        res.z = stof(strZ);

        return res;
    }

    template<typename T>
    void loadVector(vector<T>& vect, const string& filePath)
    {
        ifstream file(filePath, ios::in);
        if(file.is_open())
        {
            string line;
            while(getline(file, line))
            {
                vect.push_back(readLine<T>(line));
            }
            file.close();
        }
        else
        {
            Debug::RaiseError("Could not open file : " + filePath);
        }
    }

    /* VBO indexation funtions */

    // Returns true if v1 can be considered equal to v2
    bool is_near(float v1, float v2){
        return fabs( v1-v2 ) < 0.01f;
    }

    // Searches through all already-exported vertices
    // for a similar one.
    // Similar = same position + same UVs + same normal
    bool getSimilarVertexIndex(
        glm::vec3 & in_vertex,
        glm::vec2 & in_uv,
        glm::vec3 & in_normal,
        std::vector<glm::vec3> & out_vertices,
        std::vector<glm::vec2> & out_uvs,
        std::vector<glm::vec3> & out_normals,
        unsigned short & result
    ){
        // Lame linear search
        for ( unsigned int i=0; i<out_vertices.size(); i++ ){
            if (
                is_near( in_vertex.x , out_vertices[i].x ) &&
                is_near( in_vertex.y , out_vertices[i].y ) &&
                is_near( in_vertex.z , out_vertices[i].z ) &&
                is_near( in_uv.x     , out_uvs     [i].x ) &&
                is_near( in_uv.y     , out_uvs     [i].y ) &&
                is_near( in_normal.x , out_normals [i].x ) &&
                is_near( in_normal.y , out_normals [i].y ) &&
                is_near( in_normal.z , out_normals [i].z )
            ){
                result = i;
                return true;
            }
        }
        // No other vertex could be used instead.
        // Looks like we'll have to add it to the VBO.
        return false;
    }

    void indexVBO_TBN(
        std::vector<glm::vec3> & in_vertices,
        std::vector<glm::vec2> & in_uvs,
        std::vector<glm::vec3> & in_normals,
        std::vector<glm::vec3> & in_tangents,
        std::vector<glm::vec3> & in_bitangents,

        std::vector<unsigned short> & out_indices,
        std::vector<glm::vec3> & out_vertices,
        std::vector<glm::vec2> & out_uvs,
        std::vector<glm::vec3> & out_normals,
        std::vector<glm::vec3> & out_tangents,
        std::vector<glm::vec3> & out_bitangents
    ){
        // For each input vertex
        for ( unsigned int i=0; i<in_vertices.size(); i++ ){

            // Try to find a similar vertex in out_XXXX
            unsigned short index;
            bool found = getSimilarVertexIndex(in_vertices[i], in_uvs[i], in_normals[i],     out_vertices, out_uvs, out_normals, index);

            if ( found ){ // A similar vertex is already in the VBO, use it instead !
                out_indices.push_back( index );

                // Average the tangents and the bitangents
                out_tangents[index] += in_tangents[i];
                out_bitangents[index] += in_bitangents[i];
            }else{ // If not, it needs to be added in the output data.
                out_vertices.push_back( in_vertices[i]);
                out_uvs     .push_back( in_uvs[i]);
                out_normals .push_back( in_normals[i]);
                out_tangents .push_back( in_tangents[i]);
                out_bitangents .push_back( in_bitangents[i]);
                out_indices .push_back( (unsigned short)out_vertices.size() - 1 );
            }
        }
    }

    /* Tangent space computation functions */

    void computeTangentBasisIndexed(std::vector<short unsigned int>& indices,
                                    std::vector<glm::vec3>& vertices,
                                    std::vector<glm::vec2>& uvs,
                                    std::vector<glm::vec3>& normals,
                                    std::vector<glm::vec3>& tangents,
                                    std::vector<glm::vec3>& bitangents)
    {
        //fill in the arrays (to be able to access them later)
        for (unsigned int i=0; i<vertices.size(); i++ ){
            tangents.push_back(glm::vec3(0.0));
            bitangents.push_back(glm::vec3(0.0));
        }

        for (unsigned int i=0; i<indices.size(); i+=3 ){

            // Shortcuts for vertices
            glm::vec3 & v0 = vertices[indices[i+0]];
            glm::vec3 & v1 = vertices[indices[i+1]];
            glm::vec3 & v2 = vertices[indices[i+2]];

            // Shortcuts for UVs
            glm::vec2 & uv0 = uvs[indices[i+0]];
            glm::vec2 & uv1 = uvs[indices[i+1]];
            glm::vec2 & uv2 = uvs[indices[i+2]];

            // Edges of the triangle : postion delta
            glm::vec3 deltaPos1 = v1-v0;
            glm::vec3 deltaPos2 = v2-v0;

            // UV delta
            glm::vec2 deltaUV1 = uv1-uv0;
            glm::vec2 deltaUV2 = uv2-uv0;

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
            glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

            // Set the same tangent for all three vertices of the triangle.
            // They will be merged later, in vboindexer.cpp
            tangents[indices[i+0]] = tangent;
            tangents[indices[i+1]] = tangent;
            tangents[indices[i+2]] = tangent;

            // Same thing for binormals
            bitangents[indices[i+0]] = bitangent;
            bitangents[indices[i+1]] = bitangent;
            bitangents[indices[i+2]] = bitangent;

        }

        // See "Going Further"
        for (unsigned int i=0; i<vertices.size(); i+=1 )
        {
            glm::vec3 & n = normals[i];
            glm::vec3 & t = tangents[i];
            glm::vec3 & b = bitangents[i];

            // Gram-Schmidt orthogonalize
            t = glm::normalize(t - n * glm::dot(n, t));

            // Calculate handedness
            if (glm::dot(glm::cross(n, t), b) < 0.0f){
                t = t * -1.0f;
            }

        }
    }
}


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

    string fullPath = RESSOURCE_PATH + meshFileName + "_convert";

    vector<ushort> out_indices;
    vector<vec3>   out_verts;
    vector<vec2>   out_uvs;
    vector<vec3>   out_norms;
    vector<vec3>   out_tangents;
    vector<vec3>   out_bitangents;

    string indiceFile       = fullPath + ".indices";
    string verticeFile      = fullPath + ".vertices";
    string normalFile       = fullPath + ".normals";
    string uvFile           = fullPath + ".uvs";
    string tangentFile      = fullPath + ".tangents";
    string bitangentFile    = fullPath + ".bitangents";

    loadVector<ushort>(out_indices, indiceFile);
    loadVector<vec3>(out_verts, verticeFile);
    loadVector<vec3>(out_norms, normalFile);
    loadVector<vec2>(out_uvs, uvFile);
    loadVector<vec3>(out_tangents, tangentFile);
    loadVector<vec3>(out_bitangents, bitangentFile);

    return s_instance->addMeshData(meshFileName, out_indices, out_verts, out_norms, out_uvs,
                                   out_tangents, out_bitangents);;
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
    short *angleIndices = new short[nbSteps * nbSteps];
    //set the array to -1 as default value
    for(int i = 0; i < nbSteps; ++i)
    {
        for(int j = 0; j < nbSteps; ++j)
        {
            angleIndices[i * nbSteps + j] = -1;
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

                    uint index = (xStep + stepAddX) * nbSteps + yStep + stepAddY;
                    if(angleIndices[index] < 0)
                    { //first encouter of this vertice
                        vertices.push_back(dir);
                        normals.push_back(normalizedDir);
                        uvs.push_back(vec2(u[stepAddX], v[stepAddY]));
                        angleIndices[index] = vertices.size() - 1;
                    }
                    vertIndices[indCount] = angleIndices[index];
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

    vector<vec3> tangents;
    vector<vec3> bitangents;
    computeTangentBasisIndexed(indices, vertices, uvs, normals,
                               tangents, bitangents);

    delete[] angleIndices;
    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangents, bitangents);
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
    int nbAngleSteps    = 360.0f / angleStep + 1;
    int nbLengthSteps   = length / lengthStep + 1;


    //indicies of vertices, normal and uvs stored by [angle over x, distance over z]
    short *viewedVertices = new short[nbAngleSteps * nbLengthSteps];
    //set the array to -1 as default value
    for(int i = 0; i < nbAngleSteps; ++i)
    {
        for(int j = 0; j < nbLengthSteps; ++j)
        {
            viewedVertices[i * nbLengthSteps + j] = -1;
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

    glm::quat coneRotation = glm::angleAxis(angle, vec3(1.0f, 0.0f, 0.0f));

    //Generate vertices for cone surface
    //loop over all the angles to make a full 360 around the cone's Z axis
    for(int zAngleStep = 0; zAngleStep < nbAngleSteps - 1; ++zAngleStep)
    {
        float zAngle = zAngleStep * angleStep;

        glm::quat zRot[2] =
        {
            glm::angleAxis(zAngle, 0.0f, 0.0f, 1.0f) * coneRotation,
            glm::angleAxis(zAngle + angleStep, 0.0f, 0.0f, 1.0f) * coneRotation
        };

        for(int zPosStep = 0; zPosStep < nbLengthSteps - 1; ++zPosStep)
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

                    int index = (zAngleStep + subStepZAngle) * nbLengthSteps + zPosStep + subStepZPos;
                    if(viewedVertices[index] < 0)
                    { //first encouter of this vertex
                        vertices.push_back(pos);
                        //Normal(approx) is dir from pos on z axis to pos on the surface of the cone
                        //add small offset to handle the case of the vertex at tip of the cone
                        vec3 normal = normalize(pos - vec3(0.0f, 0.0f, zPos[subStepZPos] + 0.001f));
                        normals.push_back(normal);
                        uvs.push_back(vec2(u[subStepZAngle], v[subStepZPos]));
                        viewedVertices[index] = vertices.size() - 1;
                    }
                    vertIndices[indCount] = viewedVertices[index];
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
        delete[] viewedVertices;
    }

    vector<vec3> tangents;
    vector<vec3> bitangents;
    computeTangentBasisIndexed(indices, vertices, uvs, normals,
                               tangents, bitangents);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangents, bitangents);
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

    vector<vec3> tangents;
    vector<vec3> bitangents;
    computeTangentBasisIndexed(indices, vertices, uvs, normals,
                               tangents, bitangents);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangents, bitangents);
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
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f),
           // Back face
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f),
           // Top face
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f),
           // Bottom face
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f),
           // Right face
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f),
           // Left face
           vec2(0.0f, 0.0f),
           vec2(1.0f, 0.0f),
           vec2(1.0f, 1.0f),
           vec2(0.0f, 1.0f)
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

    vector<vec3> tangents;
    vector<vec3> bitangents;
    computeTangentBasisIndexed(indices, vertices, uvs, normals,
                               tangents, bitangents);

    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangents, bitangents);
}

uint SCEMeshLoader::CreateCustomMesh(const std::vector<ushort>& indices,
                                     const std::vector<vec3>& vertices,
                                     const std::vector<vec3>& normals,
                                     const std::vector<vec2>& uvs,
                                     const std::vector<vec3>& tangents,
                                     const std::vector<vec3>& bitangents)
{
    Debug::Assert(s_instance,
                  "No Mesh loader system instance found, Init the system before using it");
    string meshName = "Custom" + std::to_string(s_instance->mNextId);
    return s_instance->addMeshData(meshName, indices, vertices, normals, uvs, tangents, bitangents);
}

void SCEMeshLoader::DeleteMesh(uint meshId)
{
    Debug::Assert(s_instance,
                  "No Mesh loader system instance found, Init the system before using it");

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
