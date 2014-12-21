/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Component.hpp ********/
/**************************************/
#ifndef SCE_COMPONENT_HPP
#define SCE_COMPONENT_HPP

#include "SCEDefines.hpp"
#include "HandleTarget.hpp"
#include "Handle.hpp"

namespace SCE {

    class Container;//forward declaration of container

    class Component : public HandleTarget {

    public :

        Component()                 = delete;
        Component(const Component&) = delete;
        Component(Component&& )     = delete;

        virtual                     ~Component();

        Handle<Container>           GetContainer();

        const Handle<Container>     GetContainer() const ;

        const int&                  GetTypeHash() const;        

    protected :

                                    Component(Handle<Container>& container, const std::string& typeName, const int& guid = 0);


    private :

        Handle<Container>   mContainer;
        int                 mTypeHash;

    };

}


#endif
