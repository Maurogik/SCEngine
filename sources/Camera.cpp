/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Camera.cpp *********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Camera.hpp"
#include "../headers/Transform.hpp"
#include <algorithm>

using namespace SCE;
using namespace std;


Camera::Camera(Container& container)
    : Component(container, "Camera::")
    , mType (PERSPECTIVE)
{
    mProjectionMatrix  = glm::perspective(
                45.0f //Fov
              , 4.0f / 3.0f //aspect ratio
              , 0.1f //near plane distance
              , 100.0f //far plane distance
        );
    init();
}

Camera::Camera(
          Container& container
        , const float &fieldOfView
        , const float &aspectRatio
        , const float &nearPlane, const float &farPlane)
    : Component(container, "Camera::")
    , mType (PERSPECTIVE)
{
    mProjectionMatrix  = glm::perspective(
                  fieldOfView
                , aspectRatio
                , nearPlane
                , farPlane
          );
    init();
}

Camera::Camera(
          Container& container,
          const float &leftPlane
        , const float &rightPlane
        , const float &topPlane
        , const float &bottomPlane
        , const float &nearPlane
        , const float &farPlane)
    : Component(container, "Camera::")
    , mType (ORTHOGRAPHIC)
{
    mProjectionMatrix = glm::ortho(
                  leftPlane
                , rightPlane
                , bottomPlane
                , topPlane
                , nearPlane
                , farPlane
            );
    init();
}

Camera::~Camera()
{

}

void Camera::init()
{
    /*
    glm::lookAt(
                glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
           );
     */

    //Needed because opengl camera renders along the negative Z axis
    mNegativeZInverter = glm::rotate(mat4(1.0f), 180.0f, vec3(0.0f, 1.0f, 0.0f));
    //mNegativeZInverter = mat4(1.0f);
    mRenderedLayers.push_back(DEFAULT_LAYER);
}

const CameraType& Camera::GetType() const
{
    return mType;
}

mat4 Camera::GetViewMatrix() const
{
    const Transform& transform = GetContainer().GetComponent<Transform>();
    return mNegativeZInverter * inverse(transform.GetWorldTransform());;
}

const mat4& Camera::GetProjectionMatrix() const
{
    return mProjectionMatrix;
}

mat4 Camera::GetViewProjectionMatrix() const
{
    return mProjectionMatrix * GetViewMatrix();
}

bool Camera::IsLayerRendered(const std::string &layer) const
{
    return std::find(begin(mRenderedLayers), end(mRenderedLayers), layer)
            != end(mRenderedLayers) ;
}

void Camera::AddLayerToRender(const std::string &layer)
{
    if( std::find(begin(mRenderedLayers), end(mRenderedLayers), layer)
            == end(mRenderedLayers) ) {
        mRenderedLayers.push_back(layer);
    }
}

void Camera::RemoveLayerToRender(const std::string &layer)
{
    std::vector<std::string>::iterator it_layer = std::find(
                  begin(mRenderedLayers)
                , end(mRenderedLayers)
                , layer
            );

    if(it_layer != end(mRenderedLayers) ) {
        mRenderedLayers.erase(it_layer);
    }
}
