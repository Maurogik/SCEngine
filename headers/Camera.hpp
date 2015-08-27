/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Camera.hpp *********/
/**************************************/
#ifndef SCE_CAMERA_HPP
#define SCE_CAMERA_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include "SCERenderStructs.hpp"

namespace SCE
{
    struct CameraRenderData;

    class Camera : public Component
    {

    public :

        ProjectionType          GetProjectionType() const;
        glm::mat4               GetViewMatrix() const;
        const glm::mat4&        GetProjectionMatrix() const;
        glm::mat4               GetViewProjectionMatrix() const;
        void                    SetProjectionMatrix(const glm::mat4 &projMat);
        bool                    IsLayerRendered(const std::string &layer) const;
        void                    AddLayerToRender(const std::string &layer);
        void                    RemoveLayerToRender(const std::string &layer);
        CameraRenderData        GetRenderData() const;
        FrustrumData            GetFrustrumData() const;

        static std::vector<vec3>   GetFrustrumCorners(FrustrumData frustrumData, mat4 camToWorld);

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

        static std::vector<vec3>                calcConersPerspective(float near,
                                                                      float far,
                                                                      float aspectRatio,
                                                                      float fov);

        static std::vector<vec3>                calcConersOrtho(float leftPlane,
                                                                float rightPlane,
                                                                float topPlane,
                                                                float bottomPlane,
                                                                float nearPlane,
                                                                float farPlane);

        void                init();


        std::vector<std::string>    mRenderedLayers;
        glm::mat4                   mProjectionMatrix;        
        FrustrumData                mFrustrumData;

    };

}

#endif
