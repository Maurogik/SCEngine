/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Handle.hpp **********/
/**************************************/
#ifndef SCE_HANDLE_HPP
#define SCE_HANDLE_HPP

#include <memory>
#include "SCEHandleTarget.hpp"
#include "SCEInternal.hpp"

namespace SCE {

    template <class T>
    class SCEHandle {
        friend class Container;

    public :

        SCEHandle();

        SCEHandle (SCEHandleTarget* target);

        SCEHandle (const SCEHandle<T>& handle);

        SCEHandle (SCEHandle<T>&& handle);

        SCEHandle<T>& operator= (const SCEHandle<T>& rhs);

        SCEHandle<T>& operator= (SCEHandleTarget* rhs);

        T*                      operator->();

        const T*                operator->() const;

        T*                      getRaw();

        explicit operator bool() const;

        bool operator== (const SCEHandle<T>& rhs);

        bool operator== (const SCEHandleTarget* rhs);

        bool operator!= (const SCEHandle<T>& rhs);

        bool operator!= (const SCEHandleTarget* rhs);

    private :

        void                    invalidateTarget();

        SCEHandleTarget*        mTarget;
        Delegate                mDestroyedDelegate;

    };

}

#include "../templates/SCEHandle.tpp"

#endif
