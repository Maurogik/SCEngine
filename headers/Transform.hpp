/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Transform.hpp ********/
/**************************************/
#ifndef SCE_TRANSFORM_HPP
#define SCE_TRANSFORM_HPP

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/constants.hpp>

#include <math.h>
#include "SCEDefines.hpp"
#include "SCETools.hpp"
#include "Component.hpp"
#include "../common/quaternion_utils.hpp"

namespace SCE {

    class Transform : public Component{

    public :

        virtual                 ~Transform();

        //--------------------- Position
        /**
         * @brief Returns the object's position relative to it's parent
         * @return a vector3 representing the local position
         */
        const glm::vec3&        GetLocalPosition() const;

        /**
         * @brief Computes and returns the object's position relative to the world origin
         * @return a vector3 representing the world position
         */
        glm::vec3               GetWorldPosition() const;

        void                    SetLocalPosition(const glm::vec3& position);
        void                    SetWorldPosition(const glm::vec3& position);

        //--------------------- Scale
        /**
         * @brief Returns the object's scale relative to it's parent's scale
         * @return a vector3 representing the local scale
         */
        const glm::vec3&        GetLocalScale() const;

        /**
         * @brief Computes and returns the object's scale in world space
         * @return a vector3 representing the world scale
         */
        glm::vec3               GetWorldScale() const;

        void                    SetLocalScale(const glm::vec3& scale);

        //--------------------- Orientation and rotation
        /**
         * @brief Computes and returns the object's orientation relative to it's parent
         * @return a vector3 representing the rotation expressed as euler angles
         */
        glm::vec3               GetLocalOrientation() const;

        /**
         * @brief Computes and returns the object's orientation in world space
         * @return a vector3 representing the rotation expressed as euler angles
         */
        glm::vec3               GetWorldOrientation() const;

        /**
         * @brief Sets the orientation of the object relative to it's parent
         * @param orientation : a vector3 representing the euler angles orientation of the object
         */
        void                    SetLocalOrientation(const glm::vec3& orientation);

        /**
         * @brief Sets the world space orientation of the object
         * @param orientation : a vector3 representing the euler angles orientation of the object
         */
        void                    SetWorldOrientation(const glm::vec3& orientation);

        /**
         * @brief Returns the object's orientation relative to it's parent
         * @return a quaternion representing the orientation
         */
        const glm::quat&        GetLocalQuaternion() const;

        /**
         * @brief Computes and returns the object's orientation in world space
         * @return a quaternion representing the orientation
         */
        glm::quat               GetWorldQuaternion() const;

        /**
         * @brief Sets the orientation of the object relative to it's parent
         * @param quaternion : a quaternion representing the orientation of the object
         */
        void                    SetLocalQuaternion(const glm::quat& quaternion);

        /**
         * @brief Sets the orientation of the object relative to it's parent
         * @param quaternion : a quaternion representing the orientation of the object
         */
        void                    SetWorldQuaternion(const glm::quat& quaternion);

        /**
         * @brief Rotates the object around the given axis for the requested angle, in world space
         * @param axis world space axis
         * @param angle rotation angle (in degree)
         */
        void                    RotateAroundAxis(const glm::vec3& axis, float angle);

        /**
         * @brief Rotates the object around a world space point and axis for the given angle
         * @param pivot : world space pivot point
         * @param axis : world space rotation axis
         * @param angle : rotation angle (in degree)
         */
        void                    RotateAroundPivot(const glm::vec3& pivot, const glm::vec3& axis, float angle);

        /**
         * @brief Rotates the objects so that it is facing the target
         * @param target : a vector3 reprensenting the target in world space
         */
        void                    LookAt(const glm::vec3& target);

        /**
         * @brief Rotates the objects so that it is facing the target
         * @param target : a vector3 reprensenting the target in world space
         * @param upVector : a vector3 representing the desired up vector for the rotated transform
         */
        void                    LookAt(const glm::vec3& target, const glm::vec3& upVector);

        //-------------------- Transform
        /**
         * @brief Computes and returns the object's transformation matrix relative to it's parent's transformation matrix
         * @return a 4x4 matrix representing the object transformation
         */
        mat4                    GetLocalTransform() const;

        /**
         * @brief Computes and returns the object's transformation matrix relative to the world's origin
         * @return a 4x4 matrix representing the object transformation
         */
        mat4                    GetWorldTransform() const;

        //-------------------- Space to space convertion
        /**
         * @brief Transforms a position from local (parent's) space to wolrd space
         * @param pos : a vector3 of the position to transform
         * @return a vector3 representing the transformed position
         */
        glm::vec3               LocalToWorldPos(const glm::vec3& pos) const;

        /**
         * @brief Transforms a direction from local (parent's) space to wolrd space
         * @param dir : a vector3 of the direction to transform
         * @return a vector3 representing the transformed direction
         */
        glm::vec3               LocalToWorldDir(const glm::vec3& dir) const;

        /**
         * @brief Transforms a position from wolrd space to local (parent's) space
         * @param pos : a vector3 of the position to transform
         * @return a vector3 representing the transformed position
         */
        glm::vec3               WorldToLocalPos(const glm::vec3& pos) const;

        /**
         * @brief Transforms a direction from world space to local (parent's) space
         * @param dir : a vector3 of the position to transform
         * @return a vector3 representing the transformed direction
         */
        glm::vec3               WorldToLocalDir(const glm::vec3& dir) const;

        //-------------------- Local space constants
        /**
         * @brief Computes and returns the local up direction vector in world space
         * @return a vector3 representing the up direction
         */
        glm::vec3               Up() const;

        /**
         * @brief Computes and returns the local left direction vector in world space
         * @return a vector3 representing the left direction
         */
        glm::vec3               Left() const;

        /**
         * @brief Computes and returns the local right direction vector in world space
         * @return a vector3 representing the right direction
         */
        glm::vec3               Right() const;

        /**
         * @brief Computes and returns the local down direction vector in world space
         * @return a vector3 representing the down direction
         */
        glm::vec3               Down() const;

        /**
         * @brief Computes and returns the local forward direction vector in world space
         * @return a vector3 representing the forward direction
         */
        glm::vec3               Forward() const;

        /**
         * @brief Computes and returns the local back direction vector in world space
         * @return a vector3 representing the back direction
         */
        glm::vec3               Back() const;

        //-------------------- Parenting
        /**
         * @brief Sets the parent of the transform, the world space transform remains the same
         * @param parentPtr the transfrom's new parent
         */
        void                    SetParent(SCEHandle<Transform>  parentPtr);

        /**
         * @brief Removes the transform's parent, the local transform is now the world transform
         */
        void                    RemoveParent();

        /**
         * @brief Makes the given transform object a child of this object
         * , the child's world transform will not change
         * @param child the transform object to add as child
         */
        void                    AddChild(SCEHandle<Transform>  child);

        /**
         * @brief Remove the givent transform object from the children list and clear it's parent
         * @param child the transform object to unparent
         */
        void                    RemoveChild(SCEHandle<Transform>  child);

    protected :

                                Transform(SCEHandle<Container>& container);

    private :

        static glm::vec3        QuatToEuler(const glm::quat& q1);

        //glm::mat4                       mTransformMatrix;
        glm::vec3                       mTranslation;
        glm::vec3                       mScale;
        glm::quat                       mOrientation;

        SCEHandle<Transform>               mParent;
        std::vector<SCEHandle<Transform> > mChildren;

    };

}


#endif
