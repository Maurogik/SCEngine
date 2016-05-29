/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshRender.hpp********/
/**************************************/
#ifndef SCE_MESH_RENDERER_HPP
#define SCE_MESH_RENDERER_HPP

#include "SCEDefines.hpp"
#include <map>
#include <vector>


namespace SCE
{

    namespace MeshRender
    {
        void                InitializeMeshRenderData(ui16 meshId);
        void                DeleteMeshRenderData(ui16 meshId);
        void                MakeMeshInstanced(ui16 meshId);
        void                SetMeshInstanceMatrices(ui16 meshId, const std::vector<mat4> &instanceMatrices,
                                             GLenum drawType);
        void                SetInstanceCustomData(ui16 meshId, void* data, uint size, GLenum drawType,
                                       uint nbComponents, GLenum componentType);
        void                DrawInstances(ui16 meshId,  const mat4& projectionMatrix,
                                          const mat4& viewMatrix);
        void                RenderMesh( ui16 meshId, const mat4& projectionMatrix,
                                        const mat4& viewMatrix, const mat4& modelMatrix);
    }
}

#endif
