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
        Camera(  float fieldOfView
               , float aspectRatio
               , float nearPlane
               , float farPlane
        );
        Camera(
                 float leftPlane
               , float rightPlane
               , float topPlane
               , float bottomPlane
               , float nearPlane
               , float farPlane
        );
        virtual         ~Camera();
        CameraType      GetType();
        glm::mat4       GetViewMatrix();
        glm::mat4       GetProjectionMatrix();
        glm::mat4       GetViewProjectionMatrix();
        bool            IsLayerRendered(std::string layer);
        void            AddLayerToRender(std::string layer);
        void            RemoveLayerToRender(std::string layer);

    private :

        void            init();

        CameraType                  mType;
        std::vector<std::string>    mRenderedLayers;
        glm::mat4                   mProjectionMatrix;
        glm::mat4                   mNegativeZInverter;

    };

}


#endif
