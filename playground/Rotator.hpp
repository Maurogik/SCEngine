#ifndef ROTATOR_HPP
#define ROTATOR_HPP

#include "../headers/SCE.hpp"

class Rotator : public SCE::GameObject {

public :

    virtual void    Update();

protected :

                    Rotator(SCE::SCEHandle<SCE::Container> container);

private :

    double mSpeed;

};

#endif
