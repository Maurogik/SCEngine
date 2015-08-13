/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/
#ifndef SCE_RENDERED_MESH_HPP
#define SCE_RENDERED_MESH_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include "Camera.hpp"


#define MAX_ATTRIB_ID 10000

namespace SCE {

    struct attrib_data{
        GLuint  dataBufferId;
        void*   buffer;
        size_t  typeSize;
        GLuint  dataLocation;
        int     type;
    };

    class Camera;

    class MeshRenderer : public Component {

    public :
                        ~MeshRenderer();
        void            Render(const SCEHandle<Camera> &cam, bool renderFullScreenQuad = false);
        void            UpdateRenderedMesh();

    protected :

                        MeshRenderer(SCEHandle<Container>& container, const std::string& typeName = "");
                        MeshRenderer(SCEHandle<Container>& container, GLuint shaderProgramID, const std::string& typeName = "");

    private :

        void            initializeGLData(GLuint programID);
        void            updateMeshData();
        void            addAttribute(
                            GLuint programID
                          , const std::string &name
                          , void* buffer
                          , size_t size
                          , int type
                          , size_t typedSize
                        );
        void            setAttribute(
                            attrib_data& data
                          , void* newBuffer
                          , size_t newSize
                          );

        GLuint                      mMVPMatrixID;
        GLuint                      mViewMatrixID;
        GLuint                      mModelMatrixID;

        //indexation buffer
        GLuint                      mIndiceBuffer;

        GLuint                      mVaoID;
        std::vector<attrib_data>    mAttributes;



    };

}


#endif
