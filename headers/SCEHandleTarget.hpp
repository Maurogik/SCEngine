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

    private :

        Event<>             mInvalidateHandleEvent;
    };

}


#endif
