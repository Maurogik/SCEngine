/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCE::Times.hpp ********/
/**************************************/
#ifndef SCE_TIME_HPP
#define SCE_TIME_HPP

//#include <time.h>

namespace SCE
{

namespace Time
{
        void     Init();
        void     CleanUp();
        void     Update();
        double   TimeInSeconds();
        double   DeltaTime();
        double   RealTimeInSeconds();
        float    GetTimeSpeed();
        void     SetTimeSpeed(float value);
}

}


#endif
