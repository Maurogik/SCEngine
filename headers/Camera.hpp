/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Camera.hpp *********/
/**************************************/
#ifndef SCE_CAMERA_HPP
#define SCE_CAMERA_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"

namespace SCE
{
    struct SCECameraData;

    class Camera : public Component
    {

    public :

        enum
        {
            PERSPECTIVE = 0,
            ORTHOGRAPHIC
        } typedef Type;

        const Type&         GetProjectionType() const;
        glm::mat4           GetViewMatrix() const;
        const glm::mat4&    GetProjectionMatrix() const;
        glm::mat4           GetViewProjectionMatrix() const;
        void                SetProjectionMatrix(const glm::mat4 &projMat);
        bool                IsLayerRendered(const std::string &layer) const;
        void                AddLayerToRender(const std::string &layer);
        void                RemoveLayerToRender(const std::string &layer);
        SCECameraData       GetRenderData() const;
        std::vector<vec3>   GetFrustrumCorners() const;

    protected :

        //Default (perspective)
        Camera(  SCEHandle<Container>& container  );
        //Perspective projection contructor
        Camera(  SCEHandle<Container>& container,
                 float fieldOfView
               , float aspectRatio
               , float nearPlane
               , float farPlane
        );
        //Orthographic projection constructor
        Camera(  SCEHandle<Container>& container,
                 float leftPlane
               , float rightPlane
               , float topPlane
               , float bottomPlane
               , float nearPlane
               , float farPlane
        );

    private :

        void                init();
        void                calcConersPerspective(float near,
                                                  float far,
                                                  float aspectRatio,
                                                  float fov);
        void                calcConersOrtho(float leftPlane,
                                            float rightPlane,
                                            float topPlane,
                                            float bottomPlane,
                                            float nearPlane,
                                            float farPlane);

        Type                        mType;
        std::vector<std::string>    mRenderedLayers;
        glm::mat4                   mProjectionMatrix;
        glm::vec3                   mFarPlaneCorners[4];
        glm::vec3                   mNearPlaneCorners[4];

    };

}

#endif
