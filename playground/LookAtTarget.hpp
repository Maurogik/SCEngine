/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:LookAtTarget.hpp*********/
/**************************************/
#ifndef LOOK_AT_TARGET_HPP
#define LOOK_AT_TARGET_HPP

#include "../headers/SCE.hpp"

class LookAtTarget : public SCE::GameObject {

public :

                    ~LookAtTarget();
    virtual void    Update();

protected :

                    LookAtTarget(SCE::SCEHandle<SCE::Container> container);

private :

    glm::vec3       mWorldTarget;

};

#endif
