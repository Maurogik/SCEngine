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
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCEHandle.hpp"

#include "../headers/Transform.hpp"

using namespace SCE;
using namespace std;

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, ui16 meshId)
    : Component(container, "MeshRenderer::"),
      mMeshId(meshId), mIsShadowCaster(true)
{ 
    SCE::MeshRender::InitializeMeshRenderData(mMeshId);
}

MeshRenderer::MeshRenderer(SCEHandle<Container> &container, const string &filename)
    : Component(container, "MeshRenderer::"),
      mMeshId(-1), mIsShadowCaster(true)
{
    mMeshId = SCE::MeshLoader::CreateMeshFromFile(filename);
    SCE::MeshRender::InitializeMeshRenderData(mMeshId);
}

void MeshRenderer::UpdateRenderedMesh(ui16 meshId)
{
    mMeshId = meshId;
    if(mMeshId != ui16(-1))
    {
        SCE::MeshRender::InitializeMeshRenderData(mMeshId);
    }
}

bool MeshRenderer::IsCastingShadow()
{
    return mIsShadowCaster;
}

void MeshRenderer::SetIsCastingShadow(bool isShadowCaster)
{
    mIsShadowCaster = isShadowCaster;
}

void MeshRenderer::Render(const CameraRenderData& renderData, bool renderFullScreenQuad)
{
    if(mMeshId != ui16(-1))
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
}

