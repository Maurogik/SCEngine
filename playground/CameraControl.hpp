#ifndef CAM_CONTROL_HPP
#define CAM_CONTROL_HPP

#include "../headers/SCE.hpp"

class CameraControl : public SCE::GameObject {

public :

    virtual void    Update();

protected :

                    CameraControl(SCE::SCEHandle<SCE::Container> container);

private :

                    float lastMouseX;
                    float lastMouseY;
};

#endif
