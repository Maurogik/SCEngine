#ifndef ROTATOR_HPP
#define ROTATOR_HPP

#include "../headers/SCE.hpp"

class Rotator : public SCE::GameObject {

public :

                    Rotator();
                    ~Rotator();
    virtual void    Update();

private :

    double mSpeed;

};

#endif
