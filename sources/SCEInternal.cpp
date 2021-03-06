/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : SCEInternal.hpp *******/
/**************************************/

#include "../headers/SCEInternal.hpp"
#include <iostream>

using namespace std;

namespace SCE {

namespace Internal {

    void Log(const string &message)
    {
#ifdef SCE_DEBUG_ENGINE
        cout << "(Internal) : " << message << endl;
#endif
    }

}

}

