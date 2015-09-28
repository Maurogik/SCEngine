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

namespace SCE {

    class Camera;
    struct CameraRenderData;

    class MeshRenderer : public Component {

    public :

        void            Render(const CameraRenderData& renderData, bool renderFullScreenQuad = false);
        void            UpdateRenderedMesh(ui16 meshId);

    protected :

                        MeshRenderer(SCEHandle<Container>& container, ui16 meshId);
                        MeshRenderer(SCEHandle<Container>& container, const std::string& filename);

    private :

        ui16            mMeshId;

    };

}


#endif
