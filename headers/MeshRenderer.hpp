/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/
#ifndef SCE_RENDERED_MESH_HPP
#define SCE_RENDERED_MESH_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include <map>

#define MAX_ATTRIB_ID 10000

namespace SCE {

    class Camera;
    struct CameraRenderData;

    class MeshRenderer : public Component {

    public :
                        ~MeshRenderer();
        void            Render(const CameraRenderData& renderData, bool renderFullScreenQuad = false);
        void            UpdateRenderedMesh();

    protected :

                        MeshRenderer(SCEHandle<Container>& container, const std::string& typeName = "");

    private :

        enum ATTRIB_TYPE
        {
            VERTEX_POSITION = 0,
            VERTEX_UV,
            VERTEX_NORMAL,
            VERTEX_TANGENT,
            VERTEX_BITANGENT,
            VERTEX_ATTRIB_COUNT
        };

        struct attrib_data
        {
            GLuint                      dataBufferId;
            void*                       buffer;
            size_t                      typeSize;
            int                         type;
        };

        //store mandatory uniforms and attributes locations per shader
        struct shader_data
        {
            GLuint attribLocations[VERTEX_ATTRIB_COUNT];
            GLuint MVPMatrixLocation;
            GLuint ViewMatrixLocation;
            GLuint ModelMatrixLocation;
        };

        void            initializeGLData();
        void            initializeShaderData(GLuint programID);
        void            updateMeshData();

        void            addAttribute(void* buffer,
                                     size_t size,
                                     int type,
                                     size_t typedSize);

        void            setAttribute(attrib_data& data,
                                     void* newBuffer,
                                     size_t newSize);

        std::map<GLuint,shader_data>    mShaderData;

        //indexation buffer
        GLuint                          mIndiceBuffer;

        GLuint                          mVaoID;
        std::vector<attrib_data>        mAttributes;

        //Mesh attributes names as they appear in the shader
        static std::string              s_AttribNames[VERTEX_ATTRIB_COUNT];



    };

}


#endif
