#ifndef ROTATOR_HPP
#define ROTATOR_HPP

#include "../headers/SCE.hpp"

class Rotator : public SCE::GameObject {

public :

                    ~Rotator();
    virtual void    Update();

protected :

                    Rotator(SCE::Handle<SCE::Container> container);

private :

    double mSpeed;

};

#endif
