/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Component.hpp ********/
/**************************************/
#ifndef SCE_COMPONENT_HPP
#define SCE_COMPONENT_HPP

#include "SCEDefines.hpp"


namespace SCE {

    class Container;//forward declaration of container

    class Component {

    public :

                        Component();
        virtual         ~Component();
        Container*      GetContainer();
        virtual void    SetContainer(Container*container);

    private :

        Container*      mContainer;

    };

}


#endif
