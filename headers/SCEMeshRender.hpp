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

    enum ATTRIBUTE_TYPE
    {
        VERTEX_POSITION = 0,
        VERTEX_UV,
        VERTEX_NORMAL,
        VERTEX_TANGENT,
        VERTEX_BITANGENT,
        VERTEX_ATTRIB_COUNT
    };

    struct AttributeData
    {
        GLuint                      dataBufferId;
        void*                       buffer;
        size_t                      nbValues;
        GLenum                      type;
    };

    //store attributes locations per shader
    struct ShaderData
    {
        GLint attribLocations[VERTEX_ATTRIB_COUNT];
    };

    //Per mesh data
    struct MeshRenderData
    {
        MeshRenderData()
            : shaderData(),
              indiceBuffer(-1),
              indiceCount(0),
              vaoID(-1),
              attributes()
        {}
        std::map<GLuint,ShaderData>     shaderData;
        GLuint                          indiceBuffer;
        GLuint                          indiceCount;
        GLuint                          vaoID;
        std::vector<AttributeData>      attributes;
    };

    class SCEMeshRender
    {
    public :

        static void             Init();
        static void             CleanUp();

        static void             InitializeMeshRenderData(uint meshId);
        static MeshRenderData&  GetMeshRenderData(uint meshId, GLuint shaderProgram);
        static void             DeleteMeshRenderData(uint meshId);

        static void             RenderMesh(ui16 meshId, const mat4& projectionMatrix,
                                           const mat4& viewMatrix, const mat4& modelMatrix);

    private :

        static SCEMeshRender*         s_instance;

        std::map<uint, MeshRenderData>  mMeshRenderData;

        SCEMeshRender();
        ~SCEMeshRender();

        void            initializeGLData(uint meshId);
        void            initializeShaderData(MeshRenderData& renderData, GLuint programID);
        void            addAttribute(MeshRenderData& renderData,
                                     void* buffer,
                                     size_t size,
                                     GLenum type,
                                     size_t nbValues);
        void            cleanupGLRenderData(MeshRenderData& renderData);
    };
}

#endif
