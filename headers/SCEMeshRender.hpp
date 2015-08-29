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
        size_t                      typeSize;
        int                         type;
    };

    //store mandatory uniforms and attributes locations per shader
    struct ShaderData
    {
        GLuint attribLocations[VERTEX_ATTRIB_COUNT];
        GLuint MVPMatrixLocation;
        GLuint ViewMatrixLocation;
        GLuint ModelMatrixLocation;
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

        SCEMeshRender();
        ~SCEMeshRender();

        static void             Init();
        static void             CleanUp();

        static void             InitializeMeshRenderData(uint meshId);
        static MeshRenderData&  GetMeshRenderData(uint meshId, GLuint shaderProgram);
        static void             DeleteMeshRenderData(uint meshId);

    private :

        static SCEMeshRender*         s_instance;

        std::map<uint, MeshRenderData>  mMeshRenderData;

        void            initializeGLData(uint meshId);
        void            initializeShaderData(MeshRenderData& renderData, GLuint programID);
        void            addAttribute(MeshRenderData& renderData,
                                     void* buffer,
                                     size_t size,
                                     int type,
                                     size_t typedSize);
        void            cleanupGLRenderData(MeshRenderData& renderData);
    };
}

#endif
