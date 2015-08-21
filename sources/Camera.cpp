/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Camera.cpp *********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Camera.hpp"
#include "../headers/Transform.hpp"
#include "../headers/SCERender.hpp"
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
                90.0f //Fov
              , 4.0f / 3.0f //aspect ratio
              , 0.1f //near plane distance
              , 100.0f //far plane distance
        );
    init();
    calcConersPerspective(0.1f, 100.0f, 4.0f / 3.0f, 90.0f);
}

Camera::Camera(SCEHandle<Container> &container
        , float fieldOfView
        , float aspectRatio
        , float nearPlane, float farPlane)
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
    calcConersPerspective(nearPlane, farPlane, aspectRatio, fieldOfView);
}

Camera::Camera(SCEHandle<Container> &container,
          float leftPlane
        , float rightPlane
        , float topPlane
        , float bottomPlane
        , float nearPlane
        , float farPlane)
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
    calcConersOrtho(leftPlane, rightPlane, topPlane,
                    bottomPlane, nearPlane, farPlane);
}

void Camera::init()
{
    mProjectionMatrix = SCERender::FixOpenGLProjectionMatrix(mProjectionMatrix);
    mRenderedLayers.push_back(DEFAULT_LAYER);
}

void Camera::calcConersPerspective(float near, float far, float aspectRatio, float fov)
{
    glm::quat fovRot    = glm::angleAxis(fov, glm::vec3(0, 1, 0));

    vec3 leftNearMiddle = fovRot * glm::vec3(0.0f, 0.0f, near);
    //grow vector to reach plane (rotation move it off the near plane)
    leftNearMiddle      = glm::normalize(leftNearMiddle)
                        * glm::sqrt(dot(leftNearMiddle ,leftNearMiddle) + near * near);

    vec3 leftFarMiddle  = fovRot * glm::vec3(0.0f, 0.0f, far);
    //same with far plane
    leftFarMiddle       = glm::normalize(leftFarMiddle)
                        * glm::sqrt(dot(leftFarMiddle ,leftFarMiddle) + far * far);

    float topNear   = leftNearMiddle.x / aspectRatio;
    float topFar    = leftFarMiddle.x / aspectRatio;

    mFarPlaneCorners[0] = glm::vec3(leftFarMiddle.x, topFar, far);
    mFarPlaneCorners[1] = glm::vec3(-leftFarMiddle.x, topFar, far);
    mFarPlaneCorners[2] = glm::vec3(leftFarMiddle.x, -topFar, far);
    mFarPlaneCorners[3] = glm::vec3(-leftFarMiddle.x, -topFar, far);

    mNearPlaneCorners[0] = glm::vec3(leftNearMiddle.x, topNear, near);
    mNearPlaneCorners[1] = glm::vec3(-leftNearMiddle.x, topNear, near);
    mNearPlaneCorners[2] = glm::vec3(leftNearMiddle.x, -topNear, near);
    mNearPlaneCorners[3] = glm::vec3(-leftNearMiddle.x, -topNear, near);
}

void Camera::calcConersOrtho(float leftPlane, float rightPlane, float topPlane, float bottomPlane, float nearPlane, float farPlane)
{
    mFarPlaneCorners[0] = glm::vec3(rightPlane, topPlane, farPlane);
    mFarPlaneCorners[1] = glm::vec3(leftPlane, topPlane, farPlane);
    mFarPlaneCorners[2] = glm::vec3(rightPlane, bottomPlane, farPlane);
    mFarPlaneCorners[3] = glm::vec3(leftPlane, bottomPlane, farPlane);

    mNearPlaneCorners[0] = glm::vec3(rightPlane, topPlane, nearPlane);
    mNearPlaneCorners[1] = glm::vec3(leftPlane, topPlane, nearPlane);
    mNearPlaneCorners[2] = glm::vec3(rightPlane, bottomPlane, nearPlane);
    mNearPlaneCorners[3] = glm::vec3(leftPlane, bottomPlane, nearPlane);
}

const Camera::Type &Camera::GetProjectionType() const
{
    return mType;
}

mat4 Camera::GetViewMatrix() const
{
    const SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();
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

void Camera::SetProjectionMatrix(const mat4& projMat)
{
    mProjectionMatrix = projMat;
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

SCECameraData Camera::GetRenderData() const
{
    SCECameraData data;
    data.viewMatrix = GetViewMatrix();
    data.projectionMatrix = GetProjectionMatrix();

    return data;
}

vector<vec3> Camera::GetFrustrumCorners() const
{
    vector<vec3> corners;
    glm::mat4 camToWorld = GetContainer()->GetComponent<Transform>()->GetWorldTransform();

    vec4 worldCorner;

    for(int i = 0;i < 4 ;++i)
    {
        worldCorner = camToWorld * vec4(mNearPlaneCorners[i], 1.0);
        corners.push_back(vec3(worldCorner.x, worldCorner.y, worldCorner.z));
    }

    for(int i = 0;i < 4 ;++i)
    {
        worldCorner = camToWorld * vec4(mFarPlaneCorners[i], 1.0);
        corners.push_back(vec3(worldCorner.x, worldCorner.y, worldCorner.z));
    }

    return corners;
}
