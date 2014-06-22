/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : MeshRenderer.cpp ******/
/**************************************/
#ifndef SCE_RENDERED_MESH_HPP
#define SCE_RENDERED_MESH_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
//#include "Material.hpp"
#include "Camera.hpp"

namespace SCE {

    REQUIRE_COMPONENT(Transform)
    REQUIRE_COMPONENT(Mesh)

    class Camera;

    class MeshRenderer : public Component {

    public :
                        MeshRenderer();
                        ~MeshRenderer();
        virtual void    SetContainer(Container* cont);
        void            Render(Camera * cam);

    protected :

    private :
        void            initializeGLData();

        //Material        mMaterial;

        //debug
        GLuint          mProgramID;

        GLuint          mMVPMatrixID;
        GLuint          mViewMatrixID;
        GLuint          mModelMatrixID;

        //Ids
        GLuint          mVertexPosID;
        GLuint          mUvID;
        GLuint          mNormalID;

        //Buffers
        GLuint          mVertexBuffer;
        GLuint          mUVBuffer;
        GLuint          mNormalBuffer;
        GLuint          mIndiceBuffer;

    };

}


#endif
