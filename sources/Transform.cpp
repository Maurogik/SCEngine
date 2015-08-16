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
    Internal::Log("Transform initialized");
}


Transform::~Transform()
{
    for(size_t i = 0; i < mChildren.size(); ++i){
        mChildren[i]->RemoveParent();
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
    return degrees(QuatToEuler(mOrientation));
}

vec3 Transform::GetWorldOrientation() const
{
    if(!mParent) {
        return GetLocalOrientation();
    } else {
        return degrees(QuatToEuler(GetWorldQuaternion()));
    }
}

const glm::quat& Transform::GetLocalQuaternion() const
{
    return mOrientation;
}

quat Transform::GetWorldQuaternion() const
{
    if(!mParent) {
        return GetLocalQuaternion();
    } else {
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
        // math :
        // newWorldOri = ParentQuat * newLocalOri
        // newLocalOri = ParentQuat -1 * newWorldOri
    }
}


void Transform::RotateAroundAxis(const vec3 &axis, float angle)
{
    vec3 locAxis(axis);
    if(mParent){
        locAxis = mParent->WorldToLocalDir(axis);
    }
    //glm angle unit is radians
    float rads = radians(angle);
    quat rotation = angleAxis(rads, locAxis);
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

    //glm angle unit is radians
    float rads = radians(angle);
    quat rotation = angleAxis(rads, locAxis);
    mOrientation = mOrientation * rotation;
    move = rotation * move;
    mTranslation -= move;
}

//in world space
void Transform::LookAt(const glm::vec3& target)
{
    LookAt(target, vec3(0.0f, 1.0f, 0.0f));
}

//in world space
void Transform::LookAt(const glm::vec3& target, const glm::vec3& upVector)
{
    /*vec3 direction = WorldToLocalPos(target);// - mTranslation;
    direction = normalize(direction);
    quat q = QuatLookAt(direction, upVector);
    mOrientation = q;*/

    SCE::Debug::LogError("Not implemented yet");
}

void Transform::SetParent(SCEHandle<Transform> parentPtr)
{
    //make a copy of current world position, scale and rotation
    vec3 wPos   = GetWorldPosition();
    quat wQuat  = GetWorldQuaternion();
    vec3 wScale = GetWorldScale();

    //change parent
    mParent = parentPtr;
    if(mParent) {
        mParent->AddChild(this);
    }

    //convert saved transform to local
    mat4 inverseTransform = inverse(GetWorldTransform());
    mTranslation = vec3(inverseTransform * vec4(wPos, 1.0f));
    mOrientation = inverse(GetWorldQuaternion()) * wQuat;
    mScale       = vec3(inverseTransform * vec4(wScale, 0.0f));
}

void Transform::RemoveParent()
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
               , "Cannont add because the child has already been added");
    mChildren.push_back(child);
    child->SetParent(this);
}

void Transform::RemoveChild(SCEHandle<Transform> child)
{
    auto it = find(begin(mChildren), end(mChildren), child);
    Debug::Assert(it != end(mChildren)
               , "Cannont remove because the transform is not a child");
    (*it)->RemoveParent();
    mChildren.erase(it);
}

vec3 Transform::QuatToEuler(const quat& q1)
{
    double heading, attitude, bank;

    double sqw = q1.w*q1.w;
    double sqx = q1.x*q1.x;
    double sqy = q1.y*q1.y;
    double sqz = q1.z*q1.z;
    double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    double test = q1.x*q1.y + q1.z*q1.w;
    if (test > 0.499*unit) { // singularity at north pole
        heading = 2 * atan2(q1.x,q1.w);
        attitude = M_PI/2;
        bank = 0;

        return vec3(heading, attitude, bank);
    }
    if (test < -0.499*unit) { // singularity at south pole
        heading = -2 * atan2(q1.x,q1.w);
        attitude = -M_PI/2;
        bank = 0;
        return vec3(heading, attitude, bank);
    }
    heading = atan2(2*q1.y*q1.w-2*q1.x*q1.z , sqx - sqy - sqz + sqw);
    attitude = asin(2*test/unit);
    bank = atan2(2*q1.x*q1.w-2*q1.y*q1.z , -sqx + sqy - sqz + sqw);

    return vec3(heading, attitude, bank);
}


