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

    /**
     * @brief Base class for all components that need to be updated every frame
     */
    class GameObject : public Component {

    public :

        virtual             ~GameObject();
        virtual void        Update() = 0;

    protected :

                            GameObject(SCEHandle<Container>& container, const std::string& typeName);

    private :

    };

}

#endif
