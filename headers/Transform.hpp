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
#include "Component.hpp"
#include "../common/quaternion_utils.hpp"

namespace SCE {

    class Transform : public Component{

    public :

                                Transform();
        virtual                 ~Transform();


        const glm::vec3&        GetLocalPosition();
        const glm::vec3         GetWorldPosition();

        const glm::vec3&        GetLocalScale();
        const glm::vec3         GetWorldScale();

        const glm::vec3         GetLocalOrientation();
        const glm::vec3         GetWorldOrientation();
        const glm::quat&        GetLocalQuaternion();
        const glm::quat         GetWorldQuaternion();

        const mat4              GetLocalTransform();
        const mat4              GetWorldTransform();


        const glm::vec3         LocalToWorldPos(const glm::vec3& pos);
        const glm::vec3         LocalToWorldDir(const glm::vec3& dir);
        const glm::vec3         WorldToLocalPos(const glm::vec3& pos);
        const glm::vec3         WorldToLocalDir(const glm::vec3& dir);


        const glm::vec3         Up();
        const glm::vec3         Left();
        const glm::vec3         Right();
        const glm::vec3         Down();
        const glm::vec3         Forward();
        const glm::vec3         Back();


        void                    SetLocalPosition(const glm::vec3& position);
        void                    SetWorldPosition(const glm::vec3& position);

        void                    SetLocalScale(const glm::vec3& scale);

        void                    SetLocalOrientation(const glm::vec3& orientation);
        void                    SetWorldOrientation(const glm::vec3& orientation);

        /**
         * @brief Rotate around the given axis for the requested angle, in world space
         * @param axis world space axis
         * @param angle rotation angle (in degree)
         */
        void                    RotateAroundAxis(const glm::vec3& axis, const float& angle);

        /**
         * @brief Rotate around a world space point and axis for the given angle
         * @param pivot world space pivot point
         * @param axis world space rotation axis
         * @param angle rotatio angle (in degree)
         */
        void                    RotateAroundPivot(const glm::vec3& pivot, const glm::vec3& axis, const float& angle);
        void                    LookAt(const glm::vec3& target);


        void                    SetParent(Transform* parentPtr);
        void                    AddChild(Transform* child);
        void                    RemoveChild(Transform* child);


    private :

        static glm::vec3        QuatToEuler(const glm::quat& q1);

        //glm::mat4                   mTransformMatrix;
        glm::vec3                   mTranslation;
        glm::vec3                   mScale;
        glm::quat                   mOrientation;

        Transform*                  mParent;
        std::vector<Transform*>     mChildren;

    };

}


#endif
