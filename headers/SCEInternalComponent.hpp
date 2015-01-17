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
    class SCEInternalComponent : public T {

        //Container is the only class allowed to create an internal component
        friend class Container;

    public :

        SCEInternalComponent()                         = delete;
        SCEInternalComponent(const SCEInternalComponent&) = delete;
        SCEInternalComponent(SCEInternalComponent&&)      = delete;


    private :

        template < class... Args >
        SCEInternalComponent(Args&&... args): T(args...){
            SCEInternalComponent<T>::sTypeHash = T::GetTypeHash();
        }

        static int sTypeHash;

    };

    template<class T>
    int SCEInternalComponent<T>::sTypeHash;

}

#endif
