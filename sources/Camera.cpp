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
      mRenderedLayers(),
      mProjectionMatrix(),
      mFrustrumData()
{
    mProjectionMatrix  = glm::perspective(
                radians(90.0f) //Fov
              , 4.0f / 3.0f //aspect ratio
              , 1.0f //near plane distance
              , 100.0f //far plane distance
        );

    mFrustrumData.type          = PERSPECTIVE;
    mFrustrumData.near          = 1.0f;
    mFrustrumData.far           = 100.0f;
    mFrustrumData.fov           = radians(90.0f);
    mFrustrumData.aspectRatio   = 4.0f / 3.0f;

    init();
}

Camera::Camera(SCEHandle<Container> &container
        , float fieldOfView
        , float aspectRatio
        , float nearPlane, float farPlane)
    : Component(container, "Camera::"),
      mRenderedLayers(),
      mProjectionMatrix(),
      mFrustrumData()
{
    mProjectionMatrix  = glm::perspective(
                  radians(fieldOfView)
                , aspectRatio
                , nearPlane
                , farPlane
          );

    mFrustrumData.type          = PERSPECTIVE;
    mFrustrumData.near          = nearPlane;
    mFrustrumData.far           = farPlane;
    mFrustrumData.fov           = radians(fieldOfView);
    mFrustrumData.aspectRatio   = aspectRatio;

    init();
}

Camera::Camera(SCEHandle<Container> &container,
          float leftPlane
        , float rightPlane
        , float topPlane
        , float bottomPlane
        , float nearPlane
        , float farPlane)
    : Component(container, "Camera::"),
      mRenderedLayers(),
      mProjectionMatrix(),
      mFrustrumData()
{
    mProjectionMatrix = glm::ortho(
                leftPlane,
                rightPlane,
                bottomPlane,
                topPlane,
                nearPlane,
                farPlane
            );

    mFrustrumData.type = ORTHOGRAPHIC;
    mFrustrumData.near = nearPlane;
    mFrustrumData.far = farPlane;
    mFrustrumData.right = rightPlane;
    mFrustrumData.left = leftPlane;
    mFrustrumData.bottom = bottomPlane;
    mFrustrumData.top = topPlane;

    init();    
}

void Camera::init()
{
    mProjectionMatrix = SCE::Render::FixOpenGLProjectionMatrix(mProjectionMatrix);
    mRenderedLayers.push_back(DEFAULT_LAYER);
}

vector<vec3> Camera::calcConersPerspective(float near, float far, float aspectRatio, float fov)
{
    glm::quat fovRot    = glm::angleAxis(fov, glm::vec3(0, 1, 0));

    vector<vec3> corners;

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

    corners.push_back( glm::vec3(leftNearMiddle.x, topNear, near) );
    corners.push_back( glm::vec3(-leftNearMiddle.x, topNear, near) );
    corners.push_back( glm::vec3(leftNearMiddle.x, -topNear, near) );
    corners.push_back( glm::vec3(-leftNearMiddle.x, -topNear, near) );

    corners.push_back( glm::vec3(leftFarMiddle.x, topFar, far) );
    corners.push_back( glm::vec3(-leftFarMiddle.x, topFar, far) );
    corners.push_back( glm::vec3(leftFarMiddle.x, -topFar, far) );
    corners.push_back( glm::vec3(-leftFarMiddle.x, -topFar, far) );

    return corners;
}

vector<vec3> Camera::calcConersOrtho(float leftPlane, float rightPlane,
                             float topPlane, float bottomPlane,
                             float nearPlane, float farPlane)
{
    vector<vec3> corners;

    corners.push_back( glm::vec3(rightPlane, topPlane, nearPlane) );
    corners.push_back( glm::vec3(leftPlane, topPlane, nearPlane) );
    corners.push_back( glm::vec3(rightPlane, bottomPlane, nearPlane) );
    corners.push_back( glm::vec3(leftPlane, bottomPlane, nearPlane) );

    corners.push_back( glm::vec3(rightPlane, topPlane, farPlane) );
    corners.push_back( glm::vec3(leftPlane, topPlane, farPlane) );
    corners.push_back( glm::vec3(rightPlane, bottomPlane, farPlane) );
    corners.push_back( glm::vec3(leftPlane, bottomPlane, farPlane) );

    return corners;
}

ProjectionType Camera::GetProjectionType() const
{
    return mFrustrumData.type;
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

CameraRenderData Camera::GetRenderData() const
{
    CameraRenderData data;
    data.viewMatrix = GetViewMatrix();
    data.projectionMatrix = GetProjectionMatrix();

    return data;
}

FrustrumData Camera::GetFrustrumData() const
{
    return mFrustrumData;
}

std::vector<vec3> Camera::GetFrustrumCorners(FrustrumData frustrumData, glm::mat4 camToWorld)
{
    vector<vec3> corners;
    if(frustrumData.type == PERSPECTIVE)
    {
        corners = calcConersPerspective(frustrumData.near,
                                     frustrumData.far,
                                     frustrumData.aspectRatio,
                                     frustrumData.fov);
    }
    else
    {
        corners = calcConersOrtho(frustrumData.left,
                               frustrumData.right,
                               frustrumData.top,
                               frustrumData.bottom,
                               frustrumData.near,
                               frustrumData.far);
    }

    vec4 worldCorner;

    for(vec3& corner : corners)
    {
        worldCorner = camToWorld * vec4(corner, 1.0);
        corner = vec3(worldCorner.x, worldCorner.y, worldCorner.z);
    }

    return corners;
}
