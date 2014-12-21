/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Handle.hpp **********/
/**************************************/
#ifndef SCE_HANDLE_HPP
#define SCE_HANDLE_HPP

#include <memory>
#include "HandleTarget.hpp"

namespace SCE {

    template <class T>
    class Handle {
        friend class Container;

    public :

        Handle();

        Handle (HandleTarget* target);

        Handle (Handle<T>& handle);

        Handle (const Handle<T>& handle);

        Handle (Handle<T>&& handle);

        Handle<T>& operator= (const Handle<T>& rhs);

        Handle<T>& operator= (HandleTarget* rhs);

        T*                      operator->();

        const T*                operator->() const;

        explicit operator bool() const;

        bool operator== (const Handle<T>& rhs);

        bool operator!= (const Handle<T>& rhs);

    private :

        void                    invalidateTarget();

        HandleTarget*           mTarget;
        Delegate                mDestroyedDelegate;

    };

}

#include "../templates/Handle.tpp"

#endif
