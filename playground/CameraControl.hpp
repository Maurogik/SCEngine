#ifndef CAM_CONTROL_HPP
#define CAM_CONTROL_HPP

#include "../headers/SCE.hpp"

class CameraControl : public SCE::GameObject
{

public :

    virtual void    Update();

protected :

    CameraControl(SCE::SCEHandle<SCE::Container> container, SCE::SCEHandle<SCE::Transform> target);

private :

    SCE::SCEHandle<SCE::Transform>  mTarget;
    glm::vec3                       mDistanceFromTarget;

};

#endif
