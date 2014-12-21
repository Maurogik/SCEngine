/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/**** FILE : InternalComponent.hpp ****/
/**************************************/
#ifndef SCE_INTERNAL_COMPONENT_HPP
#define SCE_INTERNAL_COMPONENT_HPP

#include "SCEDefines.hpp"
#include "SCETools.hpp"
#include "Component.hpp"

namespace SCE {

    template<class T>
    class InternalComponent : public T {

        //Container is the only class allowed to create an internal component
        friend class Container;

    public :

        InternalComponent()                         = delete;
        InternalComponent(const InternalComponent&) = delete;
        InternalComponent(InternalComponent&&)      = delete;


    private :

        template < class... Args >
        InternalComponent(Args&&... args): T(args...){
            InternalComponent<T>::sTypeHash = T::GetTypeHash();
        }

        static int sTypeHash;

    };

    template<class T>
    int InternalComponent<T>::sTypeHash;

}

#endif
