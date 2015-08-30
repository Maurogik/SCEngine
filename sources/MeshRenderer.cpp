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
#include "../headers/SCEMeshRender.hpp"
#include "../headers/SCEHandle.hpp"

//import required components
#include "../headers/Transform.hpp"
#include "../headers/Mesh.hpp"
#include "../headers/Material.hpp"

using namespace SCE;
using namespace std;

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, const string &typeName)
    : Component(container, "MeshRenderer::" + typeName),
      mMeshId(-1)
{ 
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    if(mesh)
    {
        SCEMeshRender::InitializeMeshRenderData(mesh->GetMeshId());
        mMeshId = mesh->GetMeshId();
    }
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::UpdateRenderedMesh()
{
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    Debug::Assert(mesh != nullptr, "Ask to update mesh but mesh doesn't exist");
    mMeshId = mesh->GetMeshId();
    SCEMeshRender::InitializeMeshRenderData(mesh->GetMeshId());
}

void MeshRenderer::Render(const CameraRenderData& renderData, bool renderFullScreenQuad)
{
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    GLint shaderProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

    MeshRenderData& meshRenderData = SCEMeshRender::GetMeshRenderData(mMeshId, shaderProgram);
    const ShaderData &shaderData = meshRenderData.shaderData[shaderProgram];
    const vector<AttributeData>& attributes = meshRenderData.attributes;
    GLuint indiceCount = meshRenderData.indiceCount;
    GLuint indiceBuffer = meshRenderData.indiceBuffer;

    //bind mesh data
    glBindVertexArray(meshRenderData.vaoID);

    glm::mat4 projectionMatrix  = mat4(1.0);
    glm::mat4 viewMatrix        = mat4(1.0);
    glm::mat4 modelMatrix       = mat4(1.0);

    projectionMatrix  = renderData.projectionMatrix;
    viewMatrix        = renderData.viewMatrix;

    if(!renderFullScreenQuad)
    {
        modelMatrix     = transform->GetWorldTransform();
    }
    else
    {
        modelMatrix     = glm::inverse(projectionMatrix * viewMatrix);
    }

    glm::mat4 MVP       = projectionMatrix * viewMatrix * modelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(shaderData.MVPMatrixLocation, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(shaderData.ModelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(shaderData.ViewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(shaderData.ProjectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    //set the attributes
    for(size_t i = 0; i < attributes.size(); ++i)
    {
        GLuint attribLocation = shaderData.attribLocations[i];

        if(attribLocation < MAX_ATTRIB_ID)
        {
            glEnableVertexAttribArray(attribLocation);
            glBindBuffer(GL_ARRAY_BUFFER, attributes[i].dataBufferId);
            glVertexAttribPointer(
                        attribLocation,             // The attribute we want to configure
                        attributes[i].typeSize,     // size
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

    glBindVertexArray(0);

    //clean the attributes
    for(size_t i = 0; i < attributes.size(); ++i)
    {
        GLuint attribLocation = shaderData.attribLocations[i];
        if(attribLocation < MAX_ATTRIB_ID)
        {
            glDisableVertexAttribArray(attribLocation);
        }
    }

}

