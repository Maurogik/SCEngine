/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Transform.cpp ********/
/**************************************/

#include "../headers/Transform.hpp"

using namespace SCE;
using namespace std;


Transform::Transform()
    : mParent(0l)
    , mTranslation(0, 0, 0)
    , mScale(1.0f, 1.0f, 1.0f)
    , mOrientation(vec3(0, 0, 0))
{
    SCE_DEBUG_LOG("Transform initialized")
}

Transform::~Transform()
{
    for(int i = 0; i < mChildren.size(); ++i){
        mChildren[i]->SetParent(0l);
    }
    SECURE_EXEC(mParent, RemoveChild(this));
}

vec3 Transform::GetLocalPosition()
{
    //return vec3(mTransformMatrix[3]);
    return mTranslation;
}

vec3 Transform::GetWorldPosition()
{
    if(!mParent){
        return GetLocalPosition();
    }
    vec4 worldTrans = mParent->GetWorldTransform() * vec4(mTranslation, 1.0f);
    return vec3(worldTrans);
}

vec3 Transform::GetLocalScale()
{
    return mScale;
}

vec3 Transform::GetWorldScale()
{
    if(!mParent){
        return GetLocalScale();
    } else {
        mat4 worldParentTransform = mParent->GetWorldTransform();
        return vec3(worldParentTransform * vec4(mScale, 0.0f));//0 because it is a direction
    }
}

vec3 Transform::GetLocalOrientation()
{
    return QuatToEuler(mOrientation);
}

vec3 Transform::GetWorldOrientation()
{
    if(!mParent) {
        return GetLocalOrientation();
    } else {
        return QuatToEuler(GetWorldQuaternion());
    }
}

glm::quat Transform::GetLocalQuaternion()
{
    return mOrientation;
}

quat Transform::GetWorldQuaternion()
{
    if(!mParent) {
        return GetLocalQuaternion();
    } else {
        return mParent->GetWorldQuaternion() * mOrientation;
    }
}

mat4 Transform::GetLocalTransform()
{
    //1 : scale, 2 : rotate, 3 : translate
    mat4 scaleMatrix = glm::scale(mat4(1.0f), mScale);
    mat4 rotationMatrix = toMat4(mOrientation);
    mat4 tranlationMatrix = glm::translate(mat4(1.0f), mTranslation);
    return tranlationMatrix * rotationMatrix * scaleMatrix;
}

mat4 Transform::GetWorldTransform()
{
    if(!mParent){
        return GetLocalTransform();
    }
    return mParent->GetWorldTransform() * GetLocalTransform();
}

vec3 Transform::LocalToWorldPos(vec3 pos)
{
    return vec3(GetWorldTransform() * vec4(pos, 1.0f));
}

vec3 Transform::LocalToWorldDir(vec3 dir)
{
    return vec3(GetWorldTransform() * vec4(dir, 0.0f));
}

vec3 Transform::WorldToLocalPos(vec3 pos)
{
    mat4 inverseTransform = inverse(GetWorldTransform());
    return vec3(inverseTransform * vec4(pos, 1.0f));
}

vec3 Transform::WorldToLocalDir(vec3 dir)
{
    mat4 inverseTransform = inverse(GetWorldTransform());
    return vec3(inverseTransform * vec4(dir, 0.0f));
}

vec3 Transform::Up()
{
    return LocalToWorldDir(vec3(0, 1, 0));
}

vec3 Transform::Left()
{
    return LocalToWorldDir(vec3(1, 0, 0));
}

vec3 Transform::Right()
{
    return LocalToWorldDir(vec3(-1, 0, 0));
}

vec3 Transform::Down()
{
    return LocalToWorldDir(vec3(0, -1, 0));
}

vec3 Transform::Forward()
{
    return LocalToWorldDir(vec3(0, 0, 1));
}

vec3 Transform::Back()
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
    mOrientation = glm::quat(orientation);
}

void Transform::SetWorldOrientation(const vec3 &orientation)
{
    if(!mParent){
        SetLocalOrientation(orientation);
    } else {
        quat worldOrientation = quat(orientation);
        quat parentQuat = mParent->GetWorldQuaternion();
        parentQuat = inverse(parentQuat);
        mOrientation = parentQuat * worldOrientation;
// newWorldOri = ParentQuat * newLocalOri
// newLocalOri = ParentQuat -1 * newWorldOri
    }
}

/**
 * @brief Rotate around the given axis for the requested angle, in local space
 * @param axis
 * @param angle
 */
void Transform::RotateAroundAxis(vec3 axis, float angle)
{
    quat rotation = angleAxis(angle, axis);
    mOrientation = mOrientation * rotation;
}

void Transform::RotateAroundPivot(vec3 pivot, vec3 rotation)
{
    //translate to pivot, rotate, translate by inverse of 1st translation
    //in world space ?
    SCE_ASSERT(false, "Unimplemented");
}

//in world space
void Transform::LookAt(vec3 target)
{
    vec3 direction = WorldToLocalPos(target) - mTranslation;
    vec3 up = vec3(0.0f, 1.0f, 0.0f);
    quat q = QuatLookAt(direction, up);
    vec3 v = QuatToEuler(q);
    float x, y ,z;
    x = v.x;
    y = v.y;
    z = v.z;
    mOrientation = q;
}

void Transform::SetParent(Transform *parentPtr)
{
    mParent = parentPtr;
    //TODO implement parent switching without changing transform
}

void Transform::AddChild(Transform *child)
{
    SCE_ASSERT(find(mChildren.begin(), mChildren.end(), child) == mChildren.end()
               , "Cannont add because the child has already been added");
    mChildren.push_back(child);
}

void Transform::RemoveChild(Transform *child)
{
    vector<Transform*>::iterator it = find(mChildren.begin(), mChildren.end(), child);
    SCE_ASSERT(it != mChildren.end()
               , "Cannont remove because the transform is not a child");
    mChildren.erase(it);
}

vec3 Transform::QuatToEuler(quat q1)
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


