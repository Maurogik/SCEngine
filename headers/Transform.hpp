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

#include <math.h>
#include "SCEDefines.hpp"
#include "SCETools.hpp"
#include "Component.hpp"

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
        glm::vec3               GetScenePosition() const;

        void                    SetLocalPosition(const glm::vec3& position);
        void                    SetScenePosition(const glm::vec3& position);

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
        glm::vec3               GetSceneScale() const;

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
        glm::vec3               GetSceneOrientation() const;

        /**
         * @brief Sets the orientation of the object relative to it's parent
         * @param orientation : a vector3 representing the euler angles orientation of the object
         */
        void                    SetLocalOrientation(const glm::vec3& orientation);

        /**
         * @brief Sets the world space orientation of the object
         * @param orientation : a vector3 representing the euler angles orientation of the object
         */
        void                    SetSceneOrientation(const glm::vec3& orientation);

        /**
         * @brief Returns the object's orientation relative to it's parent
         * @return a quaternion representing the orientation
         */
        const glm::quat&        GetLocalQuaternion() const;

        /**
         * @brief Computes and returns the object's orientation in world space
         * @return a quaternion representing the orientation
         */
        glm::quat               GetSceneQuaternion() const;

        /**
         * @brief Sets the orientation of the object relative to it's parent
         * @param quaternion : a quaternion representing the orientation of the object
         */
        void                    SetLocalQuaternion(const glm::quat& quaternion);

        /**
         * @brief Sets the orientation of the object relative to it's parent
         * @param quaternion : a quaternion representing the orientation of the object
         */
        void                    SetSceneQuaternion(const glm::quat& quaternion);

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

        /**
         * @brief Rotate the object a fraction of the roation needed to look at the target
         * @param target : a vector3 reprensenting the target in world space
         * @param factor : how mush of the look-at rotation ot perform (1.0 = full look at)
         */
        void                    SmoothLookAt(const glm::vec3& target, float factor);

        /**
         * @brief Rotate the object a fraction of the roation needed to look at the target
         * @param target : a vector3 reprensenting the target in world space
         * @param upVector : a vector3 representing the desired up vector for the rotated transform
         * @param factor : how mush of the look-at rotation ot perform (1.0 = full look at)
         */
        void                    SmoothLookAt(const glm::vec3& target, const glm::vec3& upVector, float factor);

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
        mat4                    GetSceneTransform() const;

        //-------------------- Space to space convertion
        /**
         * @brief Transforms a position from local (parent's) space to wolrd space
         * @param pos : a vector3 of the position to transform
         * @return a vector3 representing the transformed position
         */
        glm::vec3               LocalToScenePos(const glm::vec3& pos) const;

        /**
         * @brief Transforms a direction from local (parent's) space to wolrd space
         * @param dir : a vector3 of the direction to transform
         * @return a vector3 representing the transformed direction
         */
        glm::vec3               LocalToSceneDir(const glm::vec3& dir) const;

        /**
         * @brief Transforms a position from wolrd space to local (parent's) space
         * @param pos : a vector3 of the position to transform
         * @return a vector3 representing the transformed position
         */
        glm::vec3               SceneToLocalPos(const glm::vec3& pos) const;

        /**
         * @brief Transforms a direction from world space to local (parent's) space
         * @param dir : a vector3 of the position to transform
         * @return a vector3 representing the transformed direction
         */
        glm::vec3               SceneToLocalDir(const glm::vec3& dir) const;

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

        /**
         * @brief HasParent
         * @return Returns true if the transform is a root one (no parent) and false if it is a child transform
         */
        bool                    HasParent();

    protected :

                                Transform(SCEHandle<Container>& container);

    private :

        /**
         * @brief Sets the parent of the transform, the world space transform remains the same
         * @param parentPtr the transfrom's new parent
         */
        void                    setParent(SCEHandle<Transform>  parentPtr);

        /**
         * @brief Removes the transform's parent, the local transform is now the world transform
         */
        void                    removeParent();

        //glm::mat4                       mTransformMatrix;
        glm::vec3                       mTranslation;
        glm::vec3                       mScale;
        glm::quat                       mOrientation;

        SCEHandle<Transform>               mParent;
        std::vector<SCEHandle<Transform> > mChildren;

    };

}


#endif
