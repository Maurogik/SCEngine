/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETimes.hpp ********/
/**************************************/
#ifndef SCE_TIME_HPP
#define SCE_TIME_HPP

//#include <time.h>

namespace SCE {

    class SCETime {

    public :

        static void     Init();
        static void     CleanUp();
        static void     Update();
        static double   TimeInSeconds();
        static double   DeltaTime();
        static double   RealTimeInSeconds();
        static float    GetTimeSpeed();
        static void     SetTimeSpeed(float value);

    private :

        SCETime();
        ~SCETime();

        void            update();

        static SCETime* s_instance;

        float       mTimeSpeed;
        double      mTimeInSeconds;
        double      mDeltaTime;
        double      mRealTimeInSeconds;
        double      mStartTime;
        double      mLastTime;

    };

}


#endif
