#ifndef CAM_CONTROL_HPP
#define CAM_CONTROL_HPP

#include "../headers/SCE.hpp"

class CameraControl : public SCE::GameObject
{

public :

    virtual void    Update();

protected :

    CameraControl(SCE::SCEHandle<SCE::Container> container, SCE::SCEHandle<SCE::Transform> target,
                  glm::vec3 distFromTarget);

private :

    SCE::SCEHandle<SCE::Transform>  mTarget;
    glm::vec3                       mDistanceFromTarget;
    glm::vec3                       mLookAheadTarget;
    glm::vec3                       mAverageOffset;

};

#endif
