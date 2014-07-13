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

    REQUIRE_COMPONENT(Transform)
    REQUIRE_COMPONENT(Mesh)
    REQUIRE_COMPONENT(Material)

    struct attrib_data{
        GLuint dataBuffer;
        size_t size;
        GLuint dataID;
        int type;
    };

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

        GLuint          mMVPMatrixID;
        GLuint          mViewMatrixID;
        GLuint          mModelMatrixID;

        //indexation buffer
        GLuint          mIndiceBuffer;

        std::vector<attrib_data>    mAttributes;

        void addAttribute(  const std::string &name
                          , void* buffer
                          , const size_t &size
                          , const int &type
                          , const size_t &typedSize);

    };

}


#endif
