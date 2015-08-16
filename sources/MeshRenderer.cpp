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

//import required components
#include "../headers/Transform.hpp"
#include "../headers/Mesh.hpp"
#include "../headers/Material.hpp"

using namespace SCE;
using namespace std;

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, const string &typeName)
    : Component(container, "MeshRenderer::" + typeName)
{
    SCEHandle<Material> mat = GetContainer()->GetComponent<Material>();
    initializeGLData(mat->GetShaderProgram());
}

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, GLuint shaderProgramID, const string &typeName)
    : Component(container, "MeshRenderer::" + typeName)
{
    initializeGLData(shaderProgramID);
}

MeshRenderer::~MeshRenderer()
{
    //unload all loaded data
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glDeleteBuffers(1, &mAttributes[i].dataBufferId);
    }
    glDeleteBuffers(1, &mIndiceBuffer);
}

void MeshRenderer::UpdateRenderedMesh()
{
    updateMeshData();
}

void MeshRenderer::initializeGLData(GLuint programID)
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


    /** This will be in the trasform later,...or not */
    // Get a handle for our "MVP" uniform


    mMVPMatrixUniform    = glGetUniformLocation(programID, "MVP");
    mViewMatrixUniform   = glGetUniformLocation(programID, "V");
    mModelMatrixUniform  = glGetUniformLocation(programID, "M");

    /* Allocate and assign a Vertex Array Object to our handle */
    glGenVertexArrays(1, &mVaoID);

    /* Bind our Vertex Array Object as the current used object */
    glBindVertexArray(mVaoID);

    addAttribute( programID
                , "vertexPosition_modelspace"
                , &(*vertices)[0]
                , vertices->size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    addAttribute( programID
                , "vertexUV"
                , &(*uvs)[0]
                , uvs->size() * sizeof(glm::vec2)
                , GL_FLOAT
                , 2);

    addAttribute( programID
                , "vertexNormal_modelspace"
                , &(*normals)[0]
                , normals->size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    if(tangents){
        addAttribute( programID
                    , "vertexTangent"
                    , &(*tangents)[0]
                    , tangents->size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    if(bitangents){
        addAttribute( programID
                    , "vertexBitangent"
                    , &(*bitangents)[0]
                    , bitangents->size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    glGenBuffers(1, & mIndiceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER
                 , indices->size() * sizeof(unsigned short)
                 , &(*indices)[0] , GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Disable our Vertex Array Object
    glBindVertexArray(0); // Disable our Vertex Buffer Object

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

void MeshRenderer::addAttribute(GLuint programID
                                , const std::string &name
                                , void* buffer
                                , size_t size
                                , int type
                                , size_t typedSize)
{
    GLuint id = glGetAttribLocation(programID, name.c_str());

    if(id < MAX_ATTRIB_ID){
        attrib_data attribData;
        attribData.dataLocation = id;

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
}

void MeshRenderer::setAttribute(attrib_data& data, void* newBuffer, size_t newSize)
{
    glBindBuffer(GL_ARRAY_BUFFER, data.dataBufferId);
    glBufferData(GL_ARRAY_BUFFER
                 , newSize
                 , newBuffer, GL_STATIC_DRAW);

    data.buffer = newBuffer;
}

void MeshRenderer::Render(const SCEHandle<Camera>& cam, bool renderFullScreenQuad)
{
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();    

    vector<ushort>* indices     = mesh->GetIndices();

    //bind mesh data
    glBindVertexArray(mVaoID);

    glm::mat4 projectionMatrix  = mat4(1.0);
    glm::mat4 viewMatrix        = mat4(1.0);
    glm::mat4 modelMatrix       = mat4(1.0);

    if(!renderFullScreenQuad){
        projectionMatrix  = cam->GetProjectionMatrix();
        viewMatrix        = cam->GetViewMatrix();
        modelMatrix       = transform->GetWorldTransform();
    }

    glm::mat4 MVP           = projectionMatrix * viewMatrix * modelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(mMVPMatrixUniform, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(mModelMatrixUniform, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(mViewMatrixUniform, 1, GL_FALSE, &viewMatrix[0][0]);

    //set the attributes
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glEnableVertexAttribArray(mAttributes[i].dataLocation);
        glBindBuffer(GL_ARRAY_BUFFER, mAttributes[i].dataBufferId);
        glVertexAttribPointer(
                    mAttributes[i].dataLocation,// The attribute we want to configure
                    mAttributes[i].typeSize,    // size
                    mAttributes[i].type,        // type
                    GL_FALSE,                   // normalized?
                    0,                          // stride
                    (void*)0                    // array buffer offset
                    );
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
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glDisableVertexAttribArray(mAttributes[i].dataLocation);
    }

}

