/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : SCEMInternal.hpp ******/
/**************************************/
#ifndef SCE_INTERNAL_HPP
#define SCE_INTERNAL_HPP

#include "SCEDefines.hpp"

#include <stdio.h>
#include <string>


#define SECURE_DELETE(ptr) \
    if((ptr)) { \
        delete((ptr));\
        ptr = 0l; \
    }


namespace SCE {

namespace SCEInternal {

    void        InternalMessage(const std::string& message);

}


}

#endif
