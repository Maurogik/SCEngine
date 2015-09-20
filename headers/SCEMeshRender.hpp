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
        struct MeshRenderData;

        void                InitializeMeshRenderData(ui16 meshId);
        MeshRenderData&     GetMeshRenderData(ui16 meshId, GLuint shaderProgram);
        void                DeleteMeshRenderData(ui16 meshId);

        void                RenderMesh( ui16 meshId, const mat4& projectionMatrix,
                                        const mat4& viewMatrix, const mat4& modelMatrix);
    }
}

#endif
