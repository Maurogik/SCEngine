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


Camera::Camera(SCEHandle<Container> &container)
    : Component(container, "Camera::"),
      mType (PERSPECTIVE),
      mRenderedLayers(),
      mProjectionMatrix()
{
    mProjectionMatrix  = glm::perspective(
                45.0f //Fov
              , 4.0f / 3.0f //aspect ratio
              , 0.1f //near plane distance
              , 100.0f //far plane distance
        );
    init();
}

Camera::Camera(SCEHandle<Container> &container
        , const float &fieldOfView
        , const float &aspectRatio
        , const float &nearPlane, const float &farPlane)
    : Component(container, "Camera::"),
      mType (PERSPECTIVE),
      mRenderedLayers(),
      mProjectionMatrix()
{
    mProjectionMatrix  = glm::perspective(
                  fieldOfView
                , aspectRatio
                , nearPlane
                , farPlane
          );
    init();
}

Camera::Camera(SCEHandle<Container> &container,
          const float &leftPlane
        , const float &rightPlane
        , const float &topPlane
        , const float &bottomPlane
        , const float &nearPlane
        , const float &farPlane)
    : Component(container, "Camera::"),
      mType (ORTHOGRAPHIC),
      mRenderedLayers(),
      mProjectionMatrix()
{
    mProjectionMatrix = glm::ortho(
                leftPlane,
                rightPlane,
                bottomPlane,
                topPlane,
                nearPlane,
                farPlane
            );
    init();
}

void Camera::init()
{
    //Needed because opengl camera renders along the negative Z axis and I want it to render along the positiv axis
    mProjectionMatrix = glm::scale(mProjectionMatrix, vec3(1, 1, -1));
    mRenderedLayers.push_back(DEFAULT_LAYER);
}

const Camera::Type &Camera::GetProjectionType() const
{
    return mType;
}

mat4 Camera::GetViewMatrix() const
{

    const SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();

    mat4 rotationMat = toMat4(transform->GetWorldQuaternion());
    mat4 translationMatrix = translate(mat4(1.0f), transform->GetWorldPosition());
    return inverse(transform->GetWorldTransform());
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
    if(std::find(begin(mRenderedLayers), end(mRenderedLayers), layer) == end(mRenderedLayers))
    {
        mRenderedLayers.push_back(layer);
    }
}

void Camera::RemoveLayerToRender(const std::string &layer)
{
    auto iterLayer = std::find(
                  begin(mRenderedLayers)
                , end(mRenderedLayers)
                , layer
            );

    if(iterLayer != end(mRenderedLayers))
    {
        mRenderedLayers.erase(iterLayer);
    }
}
