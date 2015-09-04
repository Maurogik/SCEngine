/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshRender.cpp********/
/**************************************/


#include "../headers/SCEMeshRender.hpp"
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"

using namespace SCE;
using namespace std;


string attribNames[5] =
{
    "vertexPosition_modelspace",
    "vertexUV",
    "vertexNormal_modelspace",
    "vertexTangent",
    "vertexBitangent"
};


SCEMeshRender* SCEMeshRender::s_instance = nullptr;

SCEMeshRender::SCEMeshRender()
    : mMeshRenderData()
{

}

SCEMeshRender::~SCEMeshRender()
{
    Internal::Log("Cleaning up mesh render system, will delete Vaos and Vbos");
    auto beginIt = begin(s_instance->mMeshRenderData);
    auto endIt = end(s_instance->mMeshRenderData);
    for(auto iterator = beginIt; iterator != endIt; iterator++) {
        cleanupGLRenderData(iterator->second);
    }
}

void SCEMeshRender::Init()
{
    Debug::Assert(!s_instance, "An instance of the Mesh render system already exists");
    s_instance = new SCEMeshRender();
}

void SCEMeshRender::CleanUp()
{
    Debug::Assert(s_instance, "No Mesh render system instance found, Init the system before using it");
    delete s_instance;
}

void SCEMeshRender::InitializeMeshRenderData(uint meshId)
{
    Debug::Assert(s_instance, "No Mesh render system instance found, Init the system before using it");

    //render data for this mesh are not initalized yet
    if(s_instance->mMeshRenderData.count(meshId) == 0)
    {
        s_instance->initializeGLData(meshId);
    }
}

MeshRenderData& SCEMeshRender::GetMeshRenderData(uint meshId, GLuint shaderProgram)
{
    Debug::Assert(s_instance, "No Mesh render system instance found, Init the system before using it");
    Debug::Assert(s_instance->mMeshRenderData.count(meshId) > 0, "Render data for mesh : " + to_string(meshId)
                  + " not initialized yet");

    MeshRenderData& renderData = s_instance->mMeshRenderData[meshId];

    if(renderData.shaderData.count(shaderProgram) == 0)
    {
        s_instance->initializeShaderData(renderData, shaderProgram);
    }
    return renderData;
}

void SCEMeshRender::DeleteMeshRenderData(uint meshId)
{
    Debug::Assert(s_instance, "No Mesh render system instance found, Init the system before using it");
    Debug::Assert(s_instance->mMeshRenderData.count(meshId) > 0, "Render data for mesh : " + to_string(meshId)
                  + " not initialized yet");

    MeshRenderData& renderData = s_instance->mMeshRenderData[meshId];

    s_instance->cleanupGLRenderData(renderData);

    auto it = s_instance->mMeshRenderData.find(meshId);
    s_instance->mMeshRenderData.erase(it);
}

void SCEMeshRender::RenderMesh(ui16 meshId,
                               const glm::mat4 &projectionMatrix,
                               const glm::mat4 &viewMatrix,
                               const glm::mat4 &modelMatrix)
{
    GLint shaderProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

    MeshRenderData& meshRenderData = SCEMeshRender::GetMeshRenderData(meshId, shaderProgram);
    const ShaderData &shaderData = meshRenderData.shaderData[shaderProgram];
    const vector<AttributeData>& attributes = meshRenderData.attributes;
    GLuint indiceCount = meshRenderData.indiceCount;
    GLuint indiceBuffer = meshRenderData.indiceBuffer;

    //bind mesh data
    glBindVertexArray(meshRenderData.vaoID);

    glm::mat4 MVP       = projectionMatrix * viewMatrix * modelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(shaderData.MVPMatrixLocation, 1, GL_FALSE, &(MVP[0][0]));
    glUniformMatrix4fv(shaderData.ModelMatrixLocation, 1, GL_FALSE, &(modelMatrix[0][0]));
    glUniformMatrix4fv(shaderData.ViewMatrixLocation, 1, GL_FALSE, &(viewMatrix[0][0]));
    glUniformMatrix4fv(shaderData.ProjectionMatrixLocation, 1, GL_FALSE, &(projectionMatrix[0][0]));

    //set the attributes
    for(size_t i = 0; i < attributes.size(); ++i)
    {
        GLuint attribLocation = shaderData.attribLocations[i];

        if(attribLocation < (GLuint)-1)
        {
            glEnableVertexAttribArray(attribLocation);
            glBindBuffer(GL_ARRAY_BUFFER, attributes[i].dataBufferId);
            glVertexAttribPointer(
                        attribLocation,             // The attribute we want to configure
                        attributes[i].nbValues,     // size
                        attributes[i].type,         // typeC
                        GL_FALSE,                   // normalized?
                        0,                          // stride
                        (void*)0                    // array buffer offset
                        );
        }
    }

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);

    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,       // mode
                indiceCount,       // count
                GL_UNSIGNED_SHORT,  // type
                (void*)0            // element array buffer offset
                );

    //clean the attributes
    for(size_t i = 0; i < attributes.size(); ++i)
    {
        GLuint attribLocation = shaderData.attribLocations[i];
        if(attribLocation < (GLuint)-1)
        {
            glDisableVertexAttribArray(attribLocation);
        }
    }

    glBindVertexArray(0);

}

void SCEMeshRender::initializeGLData(uint meshId)
{
    Internal::Log("Initializing mesh renderer data");

    const MeshData& meshData = SCEMeshLoader::GetMeshData(meshId);

    GLuint vaoId;
    /* Allocate and assign a Vertex Array Object*/
    glGenVertexArrays(1, &vaoId);

    /* Bind the Vertex Array Object as the current used object */
    glBindVertexArray(vaoId);

    //initialize and get reference at the same time
    MeshRenderData &renderData = mMeshRenderData[meshId];

    renderData.indiceCount = meshData.indices.size();
    renderData.vaoID = vaoId;

    //vertex positions
    addAttribute(   renderData,
                    (void*)(&(meshData.vertices[0])),
                    meshData.vertices.size() * sizeof(glm::vec3),
                    GL_FLOAT,
                    3);

    //vertex uvs
    addAttribute( renderData,
                  (void*)(&(meshData.uvs[0]))
                , meshData.uvs.size() * sizeof(glm::vec2)
                , GL_FLOAT
                , 2);

    //vertex normals
    addAttribute( renderData,
                  (void*)(&(meshData.normals[0]))
                , meshData.normals.size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    //vertex tangents
    if(meshData.tangents.size() > 0){
        addAttribute( renderData,
                      (void*)(&(meshData.tangents[0]))
                    , meshData.tangents.size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    //vertex bitangents
    if(meshData.bitangents.size() > 0){
        addAttribute( renderData,
                      (void*)(&(meshData.bitangents[0]))
                    , meshData.bitangents.size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    GLuint indiceBuffer;
    glGenBuffers(1, &indiceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER
                 , meshData.indices.size() * sizeof(unsigned short)
                 , &(meshData.indices[0]), GL_STATIC_DRAW);

    renderData.indiceBuffer = indiceBuffer;

    glEnableVertexAttribArray(0); // Disable the Vertex Buffer Object
    glBindVertexArray(0); // Disable the Vertex Array Object
}

void SCEMeshRender::initializeShaderData(MeshRenderData& renderData, GLuint programID)
{
    ShaderData& data = renderData.shaderData[programID];

    data.MVPMatrixLocation          = glGetUniformLocation(programID, "MVP");
    data.ViewMatrixLocation         = glGetUniformLocation(programID, "V");
    data.ModelMatrixLocation        = glGetUniformLocation(programID, "M");
    data.ProjectionMatrixLocation   = glGetUniformLocation(programID, "P");

    //preload attribute locations
    for(int i = 0; i < VERTEX_ATTRIB_COUNT; ++i)
    {
        GLuint id = glGetAttribLocation(programID, attribNames[i].c_str());
        data.attribLocations[i] = id;
    }
}

void SCEMeshRender::addAttribute(MeshRenderData& renderData,
                                 void* buffer,
                                 size_t size,
                                 GLenum type,
                                 size_t nbValues)
{
    AttributeData attribData;

    glGenBuffers(1, &attribData.dataBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, attribData.dataBufferId);
    glBufferData(GL_ARRAY_BUFFER
                 , size
                 , buffer, GL_STATIC_DRAW);

    attribData.type = type;
    attribData.nbValues = nbValues;
    attribData.buffer = buffer;
    renderData.attributes.push_back(attribData);
}

void SCEMeshRender::cleanupGLRenderData(MeshRenderData& renderData)
{
    for(size_t i = 0; i < renderData.attributes.size(); ++i){
        glDeleteBuffers(1, &(renderData.attributes[i].dataBufferId));
    }
    glDeleteBuffers(1, &(renderData.indiceBuffer));
    glDeleteVertexArrays(1, &(renderData.vaoID));
}
