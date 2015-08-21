/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Component.hpp ********/
/**************************************/
#ifndef SCE_COMPONENT_HPP
#define SCE_COMPONENT_HPP

#include "SCEDefines.hpp"
#include "SCEHandleTarget.hpp"
#include "SCEHandle.hpp"

namespace SCE {

    class Container;//forward declaration of container

    class Component : public SCEHandleTarget {

    public :
        //do not allow default contructors and copy operators to be generated
        Component()                             = delete;
        Component(const Component&)             = delete;
        Component(Component&&)                  = delete;
        Component& operator= (const Component&) = delete;
        Component& operator= (Component&&)      = delete;
        //declare destructor as virtual as this is the base class for other components
        virtual                     ~Component();

        SCEHandle<Container>        GetContainer();
        const SCEHandle<Container>  GetContainer() const ;
        const int&                  GetTypeHash() const;

    protected :

                                    Component(SCEHandle<Container>& container,
                                              const std::string& typeName,
                                              int guid = 0);

    private :

        SCEHandle<Container>        mContainer;
        int                         mTypeHash;
    };

}


#endif
