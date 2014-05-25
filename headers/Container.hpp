/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.hpp ********/
/**************************************/
#ifndef SCE_CONTAINER_HPP
#define SCE_CONTAINER_HPP

#include "SCEDefines.hpp"

#include <map>

namespace SCE {

    class Component;//forward declaration of component

    class Container {

    public :

                        Container();
        virtual         ~Container();
        void            AddComponent(std::string componentName, Component* component);
        Component*      GetComponent(std::string componentName);
        void            RemoveComponent(std::string componentName);
        std::string     GetTag();
        std::string     GetLayer();
        void            SetTag(std::string tag);
        void            SetLayer(std::string layer);

    private :

        std::map<std::string, Component*>       mComponentMap;
        std::string                             mTag;
        std::string                             mLayer;

    };

}


#endif
