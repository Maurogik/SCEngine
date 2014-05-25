/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/

#include "common/shader.hpp"


#include "../headers/MeshRenderer.hpp"
#include "../headers/Container.hpp"

//import required components
#include "../headers/Transform.hpp"
#include "../headers/Mesh.hpp"

using namespace SCE;
using namespace std;


MeshRenderer::MeshRenderer()
{

}

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::SetContainer(Container *cont)
{
    SCE_DEBUG_LOG("Setting mesh renderer container");
    Component::SetContainer(cont);
    initializeGLData();
}


void MeshRenderer::initializeGLData()
{
    SCE_DEBUG_LOG("Initializing mesh renderer data");

    Mesh* mesh = GET_COMPONENT(Mesh);
    SCE_ASSERT(mesh, "No mesh found, add a mesh component to the object adding a renderer\n");


    vector<ushort>* indices     = mesh->GetIndices();
    vector<vec3>*   vertices    = mesh->GetVertices();
    //optional
    vector<vec3>*   normals     = mesh->GetNormals();
    vector<vec2>*   uvs         = mesh->GetUvs();
    //vector<vec3>*   tangents    = mesh->GetTangents();
    //vector<vec3>*   bitangents  = mesh->GetBitangents();

    /** This will be int the material later, it is here for debug for now */
    // Create and compile our GLSL program from the shaders
    mProgramID = LoadShaders(   "../shaders/Debug.vertexshader"
                              , "../shaders/Debug.fragmentshader" );

    /** This will be in the trasform later */
    // Get a handle for our "MVP" uniform
    mMVPMatrixID    = glGetUniformLocation(mProgramID, "MVP");
    mViewMatrixID   = glGetUniformLocation(mProgramID, "V");
    mModelMatrixID  = glGetUniformLocation(mProgramID, "M");

    // Get a handle for our buffers
    mVertexPosID    = glGetAttribLocation(mProgramID, "vertexPosition_modelspace");
    mUvID           = glGetAttribLocation(mProgramID, "vertexUV");
    mNormalID       = glGetAttribLocation(mProgramID, "vertexNormal_modelspace");

    // Load it into a VBO

    glGenBuffers(1, & mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER
                 , vertices->size() * sizeof(glm::vec3)
                 , &(*vertices)[0], GL_STATIC_DRAW);

    glGenBuffers(1, & mUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
    glBufferData(GL_ARRAY_BUFFER
                 , uvs->size() * sizeof(glm::vec2)
                 , &(*uvs)[0], GL_STATIC_DRAW);

    glGenBuffers(1, & mNormalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
    glBufferData(GL_ARRAY_BUFFER
                 , normals->size() * sizeof(glm::vec3)
                 , &(*normals)[0], GL_STATIC_DRAW);

    glGenBuffers(1, & mIndiceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER
                 , indices->size() * sizeof(unsigned short)
                 , &(*indices)[0] , GL_STATIC_DRAW);

}

void MeshRenderer::Render(Camera *cam)
{
    Mesh* mesh = GET_COMPONENT(Mesh);
    Transform* transform = GET_COMPONENT(Transform);

    vector<ushort>* indices     = mesh->GetIndices();

    //bind mesh data and render

    // Use our shader
    glUseProgram(mProgramID);

    glm::mat4 ProjectionMatrix  = cam->GetProjectionMatrix();
    glm::mat4 ViewMatrix        = cam->GetViewMatrix();
    glm::mat4 ModelMatrix       = transform->GetWorldTransform();
    glm::mat4 MVP               = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(mMVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(mModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(mViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(mVertexPosID);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(
                mVertexPosID,               // The attribute we want to configure
                3,                          // size
                GL_FLOAT,                   // type
                GL_FALSE,                   // normalized?
                0,                          // stride
                (void*)0                    // array buffer offset
                );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(mUvID);
    glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
    glVertexAttribPointer(
                mUvID,                      // The attribute we want to configure
                2,                          // size : U+V => 2
                GL_FLOAT,                   // type
                GL_FALSE,                   // normalized?
                0,                          // stride
                (void*)0                    // array buffer offset
                );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(mNormalID);
    glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
    glVertexAttribPointer(
                mNormalID,                  // The attribute we want to configure
                3,                          // size
                GL_FLOAT,                   // type
                GL_FALSE,                   // normalized?
                0,                          // stride
                (void*)0                    // array buffer offset
                );

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndiceBuffer);

    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,        // mode
                indices->size(),    // count
                GL_UNSIGNED_SHORT,   // type
                (void*)0             // element array buffer offset
                );

}

