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


        glm::vec3               GetLocalPosition();
        glm::vec3               GetWorldPosition();

        glm::vec3               GetLocalScale();
        glm::vec3               GetWorldScale();

        glm::vec3               GetLocalOrientation();
        glm::vec3               GetWorldOrientation();
        glm::quat               GetLocalQuaternion();
        glm::quat               GetWorldQuaternion();

        mat4                    GetLocalTransform();
        mat4                    GetWorldTransform();


        glm::vec3               LocalToWorldPos(glm::vec3 pos);
        glm::vec3               LocalToWorldDir(glm::vec3 dir);
        glm::vec3               WorldToLocalPos(glm::vec3 pos);
        glm::vec3               WorldToLocalDir(glm::vec3 dir);


        glm::vec3               Up();
        glm::vec3               Left();
        glm::vec3               Right();
        glm::vec3               Down();
        glm::vec3               Forward();
        glm::vec3               Back();


        void                    SetLocalPosition(const glm::vec3& position);
        void                    SetWorldPosition(const glm::vec3& position);

        void                    SetLocalScale(const glm::vec3& scale);

        void                    SetLocalOrientation(const glm::vec3& orientation);
        void                    SetWorldOrientation(const glm::vec3& orientation);
        void                    RotateAroundAxis(glm::vec3 axis, float angle);
        void                    RotateAroundPivot(glm::vec3 pivot, glm::vec3 rotation);
        void                    LookAt(glm::vec3 target);

        //void                    SetLocalTransform(const glm::mat4& matrix);
        //void                    SetWorldTransform(const glm::mat4& matrix);


        void                    SetParent(Transform* parentPtr);
        void                    AddChild(Transform* child);
        void                    RemoveChild(Transform* child);


    private :

        static glm::vec3        QuatToEuler(glm::quat q1);

        //glm::mat4                   mTransformMatrix;
        glm::vec3                   mTranslation;
        glm::vec3                   mScale;
        glm::quat                   mOrientation;

        Transform*                  mParent;
        std::vector<Transform*>     mChildren;

    };

}


#endif
