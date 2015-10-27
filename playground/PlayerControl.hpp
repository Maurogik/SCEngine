#ifndef PLAYER_CONTROL_HPP
#define PLAYER_CONTROL_HPP

#include "../headers/SCE.hpp"

class PlayerControl : public SCE::GameObject {

public :

    virtual void    Update();

protected :

                    PlayerControl(SCE::SCEHandle<SCE::Container> container);

private :

                    float averageDx;
                    float averageDy;
                    float averageUpStr;
                    float averageHeight;
                    float lastSpeed;
                    SCE::SCEHandle<SCE::Transform> mTransform;
};

#endif
