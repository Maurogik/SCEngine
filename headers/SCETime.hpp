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
        static const double&   timeInSeconds();

        /**
         * @brief deltaTime
         * @return return the time since the last  frame, in seconds
         */
        static const double&   deltaTime();

        /**
         * @brief realTimeInSeconds
         * @return the actual time since the game have started
         */
        static const double&   realTimeInSeconds();

        /**
         * @brief Update the game time
         */
        static void            Update();

        static float           timeSpeed;
        static double   s_timeInSeconds;
        static double   s_deltaTime;
        static double   s_realTimeInSeconds;

    private :




        static double  start_time;
        static double  last_time;

    };

}


#endif
