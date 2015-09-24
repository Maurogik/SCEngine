/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Transform.cpp ********/
/**************************************/

#include "../headers/Transform.hpp"
#include "../headers/SCEInternal.hpp"

using namespace SCE;
using namespace std;


Transform::Transform(SCEHandle<Container> &container)
    : Component(container, "Transform"),
      mTranslation(0, 0, 0)
    , mScale(1.0f, 1.0f, 1.0f)
    , mOrientation(vec3(0, 0, 0))
    , mParent(nullptr)
{
//    Internal::Log("Transform initialized");
}


Transform::~Transform()
{
    for(size_t i = 0; i < mChildren.size(); ++i){
        mChildren[i]->removeParent();
    }
    if(mParent){
        mParent->RemoveChild(this);
    }
}

const vec3& Transform::GetLocalPosition() const
{
    return mTranslation;
}

vec3 Transform::GetWorldPosition() const
{
    if(!mParent){
        return GetLocalPosition();
    }
    vec4 worldTrans = mParent->GetWorldTransform() * vec4(mTranslation, 1.0f);
    return vec3(worldTrans);
}

const vec3& Transform::GetLocalScale() const
{
    return mScale;
}

vec3 Transform::GetWorldScale() const
{
    if(!mParent){
        return GetLocalScale();
    } else {
        mat4 worldParentTransform = mParent->GetWorldTransform();
        return vec3(worldParentTransform * vec4(mScale, 0.0f));//0 because it is a direction
    }
}

vec3 Transform::GetLocalOrientation() const
{
    return degrees(eulerAngles(mOrientation));
}

vec3 Transform::GetWorldOrientation() const
{
    if(!mParent)
    {
        return GetLocalOrientation();
    }
    else
    {
        return degrees(eulerAngles(GetWorldQuaternion()));
    }
}

const glm::quat& Transform::GetLocalQuaternion() const
{
    return mOrientation;
}

quat Transform::GetWorldQuaternion() const
{
    if(!mParent)
    {
        return GetLocalQuaternion();
    }
    else
    {
        return mParent->GetWorldQuaternion() * mOrientation;
    }
}

mat4 Transform::GetLocalTransform() const
{
    //1 : scale, 2 : rotate, 3 : translate
    mat4 scaleMatrix = glm::scale(mat4(1.0f), mScale);
    mat4 rotationMatrix = toMat4(mOrientation);
    mat4 tranlationMatrix = glm::translate(mat4(1.0f), mTranslation);
    return tranlationMatrix * rotationMatrix * scaleMatrix;
}

mat4 Transform::GetWorldTransform() const
{
    if(!mParent){
        return GetLocalTransform();
    }
    return mParent->GetWorldTransform() * GetLocalTransform();
}

vec3 Transform::LocalToWorldPos(const vec3 &pos) const
{
    return vec3(GetWorldTransform() * vec4(pos, 1.0f));
}

vec3 Transform::LocalToWorldDir(const vec3 &dir) const
{
    return vec3(GetWorldTransform() * vec4(dir, 0.0f));
}

vec3 Transform::WorldToLocalPos(const vec3 &pos) const
{
    mat4 inverseTransform = inverse(GetWorldTransform());
    return vec3(inverseTransform * vec4(pos, 1.0f));
}

vec3 Transform::WorldToLocalDir(const vec3 &dir) const
{
    mat4 inverseTransform = inverse(GetWorldTransform());
    return vec3(inverseTransform * vec4(dir, 0.0f));
}

vec3 Transform::Up() const
{
    return LocalToWorldDir(vec3(0, 1, 0));
}

vec3 Transform::Left() const
{
    return LocalToWorldDir(vec3(-1, 0, 0));
}

vec3 Transform::Right() const
{
    return LocalToWorldDir(vec3(1, 0, 0));
}

vec3 Transform::Down() const
{
    return LocalToWorldDir(vec3(0, -1, 0));
}

vec3 Transform::Forward() const
{
    return LocalToWorldDir(vec3(0, 0, 1));
}

vec3 Transform::Back() const
{
    return LocalToWorldDir(vec3(0, 0, -1));
}

void Transform::SetLocalPosition(const vec3 &position)
{
    mTranslation = position;
}

void Transform::SetWorldPosition(const vec3 &position)
{
    if(!mParent){
        SetLocalPosition(position);
    } else {
        mat4 parentTransform = mParent->GetWorldTransform();
        vec3 newLocalPos(glm::inverse(parentTransform) * vec4(position, 1.0f));

        SetLocalPosition(newLocalPos);
    }
}

void Transform::SetLocalScale(const vec3 &scale)
{
    mScale = scale;
}

void Transform::SetLocalOrientation(const vec3 &orientation)
{
    mOrientation = quat(radians(orientation));
}

void Transform::SetWorldOrientation(const vec3 &orientation)
{
    if(!mParent){
        SetLocalOrientation(orientation);
    } else {
        quat worldOrientation = quat(radians(orientation));
        quat parentQuat = mParent->GetWorldQuaternion();
        parentQuat = inverse(parentQuat);
        mOrientation = parentQuat * worldOrientation;
    }
}

void Transform::SetLocalQuaternion(const quat &quaternion)
{
    mOrientation = quaternion;
}

void Transform::SetWorldQuaternion(const quat &quaternion)
{
    if(!mParent)
    {
        SetLocalQuaternion(quaternion);
    }
    else
    {
        quat parentQuat = mParent->GetWorldQuaternion();
        parentQuat = inverse(parentQuat);
        mOrientation = parentQuat * quaternion;
    }
}

void Transform::RotateAroundAxis(const vec3 &axis, float angle)
{
    vec3 locAxis(axis);
    if(mParent){
        locAxis = mParent->WorldToLocalDir(axis);
    }
    quat rotation = angleAxis(radians(angle), locAxis);
    mOrientation = mOrientation * rotation;
}

void Transform::RotateAroundPivot(const glm::vec3& pivot, const glm::vec3& axis, float angle)
{
    vec3 locPivot(pivot);
    vec3 locAxis(axis);
    if(mParent){
        locPivot = mParent->WorldToLocalPos(pivot);
        locAxis = mParent->WorldToLocalDir(axis);
    }
    vec3 move = locPivot - mTranslation;
    mTranslation += move;

    quat rotation = angleAxis(radians(angle), locAxis);
    mOrientation = mOrientation * rotation;
    move = rotation * move;
    mTranslation -= move;
}

//in world space
void Transform::LookAt(const glm::vec3& target)
{
    LookAt(target, vec3(0.0f, 1.0f, 0.0f));
}

glm::quat rotateBetweenVector(const glm::vec3& start, const glm::vec3& end)
{
    float dot = glm::dot(start, end);
    quat rotation;

    if (dot > 0.9999999f)//same forward vectors
    {}
    else if (dot < -0.9999999f)//opposite vectors
    {
        rotation = glm::angleAxis(glm::pi<float>(), vec3(0.0, 1.0, 0.0));
    }
    else
    {
        glm::vec3 rotationAxis = normalize(glm::cross(start, end));
//        float rotationAngle = glm::acos(dot);
//        rotation = angleAxis(rotationAngle, rotationAxis);

        //other way without the acos
        float halfCos = sqrt((dot + 1.0f)*0.5f);
        float halfSin = sqrt(1.0f - halfCos*halfCos);
        rotation = quat(halfCos, halfSin * rotationAxis.x, halfSin * rotationAxis.y, halfSin * rotationAxis.z);
    }

    return rotation;
}

//in world space
void Transform::LookAt(const glm::vec3& target, const glm::vec3& upVector)
{
    glm::vec3 direction = WorldToLocalPos(target);
    direction = normalize(direction);
    glm::vec3 forward(0.0, 0.0, 1.0);
    glm::vec3 localUp = WorldToLocalDir(upVector);

    quat rotation = rotateBetweenVector(forward, direction);

    glm::vec3 right = normalize(glm::cross(direction, localUp));
    glm::vec3 desiredUp = normalize(glm::cross(right, direction));
    glm::vec3 newUp = rotation * glm::vec3(0.0, 1.0, 0.0);

    quat upRotation = rotateBetweenVector(newUp, desiredUp);

    mOrientation = mOrientation * upRotation * rotation;
}

void Transform::setParent(SCEHandle<Transform> parentPtr)
{
    //make a copy of current world position, scale and rotation
    vec3 wPos   = GetWorldPosition();
    quat wQuat  = GetWorldQuaternion();
    vec3 wScale = GetWorldScale();

    //change parent
    mParent = parentPtr;

    //convert saved transform to local
    mat4 inverseTransform = inverse(GetWorldTransform());
    mTranslation = vec3(inverseTransform * vec4(wPos, 1.0f));
    mOrientation = inverse(GetWorldQuaternion()) * wQuat;
    mScale       = vec3(inverseTransform * vec4(wScale, 0.0f));
}

void Transform::removeParent()
{
    //make a copy of current world position, scale and rotation
    vec3 wPos   = GetWorldPosition();
    quat wQuat  = GetWorldQuaternion();
    vec3 wScale = GetWorldScale();

    //chang parent
    mParent = nullptr;

    //convert save transform to local
    mTranslation = wPos;
    mOrientation = wQuat;
    mScale       = wScale;
}

void Transform::AddChild(SCEHandle<Transform> child)
{
    Debug::Assert(find(mChildren.begin(), mChildren.end(), child) == mChildren.end()
               , "Cannont add because the child was added");
    mChildren.push_back(child);
    child->setParent(this);
}

void Transform::RemoveChild(SCEHandle<Transform> child)
{
    auto it = find(begin(mChildren), end(mChildren), child);
    Debug::Assert(it != end(mChildren)
               , "Cannont remove because the transform is not a child");
    (*it)->removeParent();
    mChildren.erase(it);
}


