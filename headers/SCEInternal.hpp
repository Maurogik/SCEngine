/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : SCEMInternal.hpp ******/
/**************************************/
#ifndef SCE_INTERNAL_HPP
#define SCE_INTERNAL_HPP

#include <string>
#include "SCEDefines.hpp"

#define SECURE_DELETE(ptr) \
    if((ptr)) { \
        delete((ptr));\
        ptr = 0l; \
    }


namespace SCE {

namespace Internal {

    void        Log(const std::string& message);

}


}

#endif
