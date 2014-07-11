/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : GameObject.hpp ********/
/**************************************/
#ifndef SCE_GAME_OBJECT_HPP
#define SCE_GAME_OBJECT_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"

namespace SCE {

    class GameObject : public Component {

    public :

                            GameObject();
        virtual             ~GameObject();
        virtual void        Update() = 0;
        virtual void        SetContainer(Container* cont);

    private :

    };

}

#endif
