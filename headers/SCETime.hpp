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

        /**
         * @brief timeInSeconds
         * @return the time since the game have started
         */
        static const double&    TimeInSeconds();

        /**
         * @brief deltaTime
         * @return return the time since the last  frame, in seconds
         */
        static const double&    DeltaTime();

        /**
         * @brief realTimeInSeconds
         * @return the actual time since the game have started
         */
        static const double&    RealTimeInSeconds();

        /**
         * @brief Update the game time
         */
        static void             Update();


        static const float&     TimeSpeed();
        static void             SetTimeSpeed(float value);

    private :

        static float    s_timeSpeed;
        static double   s_timeInSeconds;
        static double   s_deltaTime;
        static double   s_realTimeInSeconds;

        static double  start_time;
        static double  last_time;

    };

}


#endif
