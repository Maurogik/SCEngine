/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/

#include "common/shader.hpp"


#include "../headers/MeshRenderer.hpp"
#include "../headers/Container.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCERender.hpp"

//import required components
#include "../headers/Transform.hpp"
#include "../headers/Mesh.hpp"
#include "../headers/Material.hpp"

using namespace SCE;
using namespace std;

string MeshRenderer::s_AttribNames[MeshRenderer::VERTEX_ATTRIB_COUNT] =
{
    "vertexPosition_modelspace",
    "vertexUV",
    "vertexNormal_modelspace",
    "vertexTangent",
    "vertexBitangent"
};

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, const string &typeName)
    : Component(container, "MeshRenderer::" + typeName)
{ 
    initializeGLData();
}

MeshRenderer::~MeshRenderer()
{
    //unload all loaded data
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glDeleteBuffers(1, &mAttributes[i].dataBufferId);
    }
    glDeleteBuffers(1, &mIndiceBuffer);
    glDeleteVertexArrays(1, &mVaoID);
}

void MeshRenderer::UpdateRenderedMesh()
{
    updateMeshData();
}

void MeshRenderer::initializeGLData()
{
    Internal::Log("Initializing mesh renderer data");

    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();

    vector<ushort>* indices     = mesh->GetIndices();
    vector<vec3>*   vertices    = mesh->GetVertices();
    //optional
    vector<vec3>*   normals     = mesh->GetNormals();
    vector<vec2>*   uvs         = mesh->GetUvs();
    vector<vec3>*   tangents    = mesh->GetTangents();
    vector<vec3>*   bitangents  = mesh->GetBitangents();

    /* Allocate and assign a Vertex Array Object*/
    glGenVertexArrays(1, &mVaoID);

    /* Bind the Vertex Array Object as the current used object */
    glBindVertexArray(mVaoID);

    //vertex positions
    addAttribute( &(*vertices)[0]
                , vertices->size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    //vertex uvs
    addAttribute( &(*uvs)[0]
                , uvs->size() * sizeof(glm::vec2)
                , GL_FLOAT
                , 2);

    //vertex normals
    addAttribute( &(*normals)[0]
                , normals->size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    //vertex tangents
    if(tangents){
        addAttribute( &(*tangents)[0]
                    , tangents->size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    //vertex bitangents
    if(bitangents){
        addAttribute( &(*bitangents)[0]
                    , bitangents->size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    glGenBuffers(1, & mIndiceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER
                 , indices->size() * sizeof(unsigned short)
                 , &(*indices)[0] , GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Disable the Vertex Buffer Object
    glBindVertexArray(0); // Disable the Vertex Array Object

}

void MeshRenderer::initializeShaderData(GLuint programID)
{
    shader_data data;

    data.MVPMatrixLocation    = glGetUniformLocation(programID, "MVP");
    data.ViewMatrixLocation   = glGetUniformLocation(programID, "V");
    data.ModelMatrixLocation  = glGetUniformLocation(programID, "M");

    //preload attribute locations
    for(int i = 0; i < VERTEX_ATTRIB_COUNT; ++i)
    {
        GLuint id = glGetAttribLocation(programID, s_AttribNames[i].c_str());
        data.attribLocations[i] = id;
    }

    mShaderData[programID] = data;
}

void MeshRenderer::updateMeshData()
{
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();

    vector<ushort>* indices     = mesh->GetIndices();
    vector<vec3>*   vertices    = mesh->GetVertices();
    //optional
    vector<vec3>*   normals     = mesh->GetNormals();
    vector<vec2>*   uvs         = mesh->GetUvs();
    vector<vec3>*   tangents    = mesh->GetTangents();
    vector<vec3>*   bitangents  = mesh->GetBitangents();

    vector<size_t> newSizes;
    vector<void*> newBuffers;

    newBuffers.push_back( &(*vertices)[0] );
    newBuffers.push_back( &(*uvs)[0] );
    newBuffers.push_back( &(*normals)[0] );

    newSizes.push_back(sizeof(glm::vec3) * vertices->size());
    newSizes.push_back(sizeof(glm::vec3) * uvs->size());
    newSizes.push_back(sizeof(glm::vec3) * normals->size());

    if(tangents)
    {
        newBuffers.push_back( &(*tangents)[0] );
        newSizes.push_back(sizeof(glm::vec3) * tangents->size());
    }
    if(bitangents)
    {
        newBuffers.push_back( &(*bitangents)[0] );
        newSizes.push_back(sizeof(glm::vec3) * bitangents->size());
    }

    /* Bind our Vertex Array Object as the current used object */
    glBindVertexArray(mVaoID);

    for(size_t i = 0; i < mAttributes.size(); ++i)
    {
        setAttribute( mAttributes[i]
                    , newBuffers[i]
                    , newSizes[i]);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER
                 , indices->size() * sizeof(unsigned short)
                 , &(*indices)[0] , GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Disable our Vertex Array Object
    glBindVertexArray(0); // Disable our Vertex Buffer Object
}

void MeshRenderer::addAttribute(void* buffer
                                , size_t size
                                , int type
                                , size_t typedSize)
{
    attrib_data attribData;

    glGenBuffers(1, &attribData.dataBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, attribData.dataBufferId);
    glBufferData(GL_ARRAY_BUFFER
                 , size
                 , buffer, GL_STATIC_DRAW);

    attribData.type = type;
    attribData.typeSize= typedSize;
    attribData.buffer = buffer;
    mAttributes.push_back(attribData);
}

void MeshRenderer::setAttribute(attrib_data& data, void* newBuffer, size_t newSize)
{
    glBindBuffer(GL_ARRAY_BUFFER, data.dataBufferId);
    glBufferData(GL_ARRAY_BUFFER
                 , newSize
                 , newBuffer, GL_STATIC_DRAW);

    data.buffer = newBuffer;
}

void MeshRenderer::Render(const CameraRenderData& renderData, bool renderFullScreenQuad)
{
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    GLint shaderProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

    //we need to check this shader was initialized
    if(mShaderData.count(shaderProgram) == 0)
    {
        initializeShaderData(shaderProgram);
    }

    const shader_data &shaderData = mShaderData[shaderProgram];

    vector<ushort>* indices     = mesh->GetIndices();

    //bind mesh data
    glBindVertexArray(mVaoID);

    glm::mat4 projectionMatrix  = mat4(1.0);
    glm::mat4 viewMatrix        = mat4(1.0);
    glm::mat4 modelMatrix       = mat4(1.0);

    if(!renderFullScreenQuad)
    {
        projectionMatrix  = renderData.projectionMatrix;
        viewMatrix        = renderData.viewMatrix;
        modelMatrix       = transform->GetWorldTransform();
    }
    else
    {
        //trick to keep a usable view matrix in shader
        viewMatrix        = renderData.viewMatrix;
        modelMatrix       = glm::inverse(renderData.viewMatrix);
    }

    glm::mat4 MVP           = projectionMatrix * viewMatrix * modelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(shaderData.MVPMatrixLocation, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(shaderData.ModelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(shaderData.ViewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

    //set the attributes
    for(size_t i = 0; i < mAttributes.size(); ++i)
    {
        GLuint attribLocation = shaderData.attribLocations[i];

        if(attribLocation < MAX_ATTRIB_ID)
        {
            glEnableVertexAttribArray(attribLocation);
            glBindBuffer(GL_ARRAY_BUFFER, mAttributes[i].dataBufferId);
            glVertexAttribPointer(
                        attribLocation,             // The attribute we want to configure
                        mAttributes[i].typeSize,    // size
                        mAttributes[i].type,        // type
                        GL_FALSE,                   // normalized?
                        0,                          // stride
                        (void*)0                    // array buffer offset
                        );
        }
    }

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);

    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,       // mode
                indices->size(),    // count
                GL_UNSIGNED_SHORT,  // type
                (void*)0            // element array buffer offset
                );

    glBindVertexArray(0);

    //clean the attributes ?
    for(size_t i = 0; i < mAttributes.size(); ++i)
    {
        GLuint attribLocation = shaderData.attribLocations[i];
        if(attribLocation < MAX_ATTRIB_ID)
        {
            glDisableVertexAttribArray(attribLocation);
        }
    }

}

