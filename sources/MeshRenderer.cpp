/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/

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


#define MAX_ATTRIB_ID 10000


using namespace SCE;
using namespace std;

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, const string &typeName)
    : Component(container, "MeshRenderer::" + typeName),
      mMeshId(-1)
{ 
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    if(mesh)
    {
        SCE::MeshRender::InitializeMeshRenderData(mesh->GetMeshId());
        mMeshId = mesh->GetMeshId();
    }
}

void MeshRenderer::UpdateRenderedMesh()
{
    SCEHandle<Mesh> mesh = GetContainer()->GetComponent<Mesh>();
    Debug::Assert(mesh != nullptr, "Ask to update mesh but mesh doesn't exist");
    mMeshId = mesh->GetMeshId();
    SCE::MeshRender::InitializeMeshRenderData(mesh->GetMeshId());
}

void MeshRenderer::Render(const CameraRenderData& renderData, bool renderFullScreenQuad)
{
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    glm::mat4 modelMatrix;

    if(renderFullScreenQuad)
    {
         modelMatrix = glm::inverse(renderData.projectionMatrix * renderData.viewMatrix);
    }
    else
    {
         modelMatrix = transform->GetWorldTransform();
    }
    SCE::MeshRender::RenderMesh(mMeshId, renderData.projectionMatrix, renderData.viewMatrix, modelMatrix);
}

