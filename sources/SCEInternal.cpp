/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******* FILE : SCEInternal.hpp *******/
/**************************************/

#include "../headers/SCEInternal.hpp"
#include <iostream>
#include <assert.h>

using namespace std;

namespace SCE {

namespace SCEInternal {

    void InternalMessage(const string &message)
    {
        cout << "(Internal) : " << message << endl;
    }

}

}

