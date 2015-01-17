/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/

#include "common/shader.hpp"


#include "../headers/MeshRenderer.hpp"
#include "../headers/Container.hpp"
#include "../headers/SCEInternal.hpp"

//import required components
#include "../headers/Transform.hpp"
#include "../headers/Mesh.hpp"
#include "../headers/Material.hpp"

using namespace SCE;
using namespace std;


MeshRenderer::MeshRenderer(SCEHandle<Container> &container, const string &typeName)
    : Component(container, "MeshRenderer::" + typeName)
{
    initializeGLData();
}

MeshRenderer::~MeshRenderer()
{
    //unload all loaded data
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glDeleteBuffers(1, &mAttributes[i].dataBuffer);
    }
    glDeleteBuffers(1, &mIndiceBuffer);
}

void MeshRenderer::initializeGLData()
{
    SCEInternal::InternalMessage("Initializing mesh renderer data");

    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();

    SCEHandle<Material> mat = GetContainer()->GetComponent<Material>();

    GLuint programID = mat->GetShaderProgram();

    vector<ushort>* indices     = mesh->GetIndices();
    vector<vec3>*   vertices    = mesh->GetVertices();
    //optional
    vector<vec3>*   normals     = mesh->GetNormals();
    vector<vec2>*   uvs         = mesh->GetUvs();
    vector<vec3>*   tangents    = mesh->GetTangents();
    vector<vec3>*   bitangents  = mesh->GetBitangents();


    /** This will be in the trasform later,...or not */
    // Get a handle for our "MVP" uniform


    mMVPMatrixID    = glGetUniformLocation(programID, "MVP");
    mViewMatrixID   = glGetUniformLocation(programID, "V");
    mModelMatrixID  = glGetUniformLocation(programID, "M");

    /* Allocate and assign a Vertex Array Object to our handle */
    glGenVertexArrays(1, &mVaoID);

    /* Bind our Vertex Array Object as the current used object */
    glBindVertexArray(mVaoID);

    addAttribute("vertexPosition_modelspace"
                , &(*vertices)[0]
                , vertices->size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    addAttribute("vertexUV"
                , &(*uvs)[0]
                , uvs->size() * sizeof(glm::vec2)
                , GL_FLOAT
                , 2);

    addAttribute("vertexNormal_modelspace"
                , &(*normals)[0]
                , normals->size() * sizeof(glm::vec3)
                , GL_FLOAT
                , 3);

    if(tangents){
        addAttribute("vertexTangent"
                    , &(*tangents)[0]
                    , tangents->size() * sizeof(glm::vec3)
                    , GL_FLOAT
                    , 3);
    }

    if(bitangents){
        addAttribute("vertexBitangent"
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

void MeshRenderer::addAttribute(  const std::string &name
                                , void* buffer
                                , const size_t &size
                                , const int &type
                                , const size_t &typedSize)
{

    SCEHandle<Material> mat = GetContainer()->GetComponent<Material>();
    GLuint programID = mat->GetShaderProgram();

    GLuint id = glGetAttribLocation(programID, name.c_str());

    if(id < MAX_ATTRIB_ID){
        attrib_data attribData;
        attribData.dataID = id;

        glGenBuffers(1, &attribData.dataBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, attribData.dataBuffer);
        glBufferData(GL_ARRAY_BUFFER
                     , size
                     , buffer, GL_STATIC_DRAW);

        attribData.type = type;
        attribData.size = typedSize;
        mAttributes.push_back(attribData);
    }
}

void MeshRenderer::Render(const SCEHandle<Camera>& cam)
{
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    SCEHandle<Material> mat = GetContainer()->GetComponent<Material>();

    vector<ushort>* indices     = mesh->GetIndices();

    //bind mesh data and render
    glBindVertexArray(mVaoID);

    // Use our shader
    mat->BindRenderData();

    glm::mat4 ProjectionMatrix  = cam->GetProjectionMatrix();
    glm::mat4 ViewMatrix        = cam->GetViewMatrix();
    glm::mat4 ModelMatrix       = transform->GetWorldTransform();
    glm::mat4 MVP               = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(mMVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(mModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(mViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);



    //set the attributes
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glEnableVertexAttribArray(mAttributes[i].dataID);
        glBindBuffer(GL_ARRAY_BUFFER, mAttributes[i].dataBuffer);
        glVertexAttribPointer(
                    mAttributes[i].dataID,               // The attribute we want to configure
                    mAttributes[i].size,                          // size
                    mAttributes[i].type,                   // type
                    GL_FALSE,                   // normalized?
                    0,                          // stride
                    (void*)0                    // array buffer offset
                    );
    }

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);

    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,        // mode
                indices->size(),    // count
                GL_UNSIGNED_SHORT,   // type
                (void*)0             // element array buffer offset
                );

    glBindVertexArray(0);

    //clean the attributes ?
    for(size_t i = 0; i < mAttributes.size(); ++i){
        glDisableVertexAttribArray(mAttributes[i].dataID);
    }

}

