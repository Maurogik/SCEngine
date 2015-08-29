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

        uint            mMeshId;

    };

}


#endif
