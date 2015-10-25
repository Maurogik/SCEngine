#ifndef ROTATOR_HPP
#define ROTATOR_HPP

#include "../headers/SCE.hpp"

class Rotator : public SCE::GameObject {

public :

    virtual void    Update();

protected :

                    Rotator(SCE::SCEHandle<SCE::Container> container, float speed, glm::vec3 axis);

private :

    float mSpeed;
    glm::vec3 mRotationAxis;

};

#endif
