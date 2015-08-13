/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : HandleTarget.hpp ******/
/**************************************/
#ifndef SCE_HANDLETARGET_HPP
#define SCE_HANDLETARGET_HPP

#include "SCEEvent.hpp"

namespace SCE {

    class SCEHandleTarget {
        template<class T> friend class SCEHandle;

    public :

        ~SCEHandleTarget()
        {   //send event to invalidate all handle for this target
            mInvalidateHandleEvent();
        }

    private :

        Event<>             mInvalidateHandleEvent;
    };

}


#endif
