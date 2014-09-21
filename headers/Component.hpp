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

        Component()                 = delete;
        Component(const Component&) = delete;
        Component(Component&& )     = delete;

        virtual                     ~Component();

        Container&                  GetContainer();

        const Container&            GetContainer() const ;

        const int&                  GetTypeHash() const;

    protected :

                                    Component(Container& container, const std::string& typeName);


    private :

        Container&    mContainer;
        int           mTypeHash;

    };

}


#endif
