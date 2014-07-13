/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Camera.hpp *********/
/**************************************/
#ifndef SCE_CAMERA_HPP
#define SCE_CAMERA_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"

namespace SCE {

    REQUIRE_COMPONENT(Transform)

    enum {
        PERSPECTIVE = 0,
        ORTHOGRAPHIC
    } typedef CameraType;


    class Camera : public Component {

    public :

        Camera();
        Camera(  const float &fieldOfView
               , const float &aspectRatio
               , const float &nearPlane
               , const float &farPlane
        );
        Camera(
                 const float &leftPlane
               , const float &rightPlane
               , const float &topPlane
               , const float &bottomPlane
               , const float &nearPlane
               , const float &farPlane
        );
        virtual             ~Camera();
        const CameraType&   GetType();
        const mat4          GetViewMatrix();
        const glm::mat4&    GetProjectionMatrix();
        const mat4          GetViewProjectionMatrix();
        bool                IsLayerRendered(const std::string &layer);
        void                AddLayerToRender(const std::string &layer);
        void                RemoveLayerToRender(const std::string &layer);

    private :

        void                init();

        CameraType                  mType;
        std::vector<std::string>    mRenderedLayers;
        glm::mat4                   mProjectionMatrix;
        glm::mat4                   mNegativeZInverter;

    };

}


#endif
