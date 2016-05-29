/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCE::Times.cpp ********/
/**************************************/

#include "../headers/SCETime.hpp"
#include <glfw3.h>

namespace SCE
{

namespace Time
{

    struct TimeData
    {
        TimeData()
            : mTimeSpeed(1.0f), mTimeInSeconds(0.0f), mDeltaTime(0.0)
        {}
        float       mTimeSpeed;
        double      mTimeInSeconds;
        double      mDeltaTime;
        double      mStartTime;
        double      mLastTime;
    };

    static TimeData globalTimeData;

    void updateTimeData(TimeData* timeData)
    {
        double currentTime = glfwGetTime();
        timeData->mDeltaTime = currentTime - timeData->mLastTime;
        timeData->mDeltaTime *= timeData->mTimeSpeed;
        timeData->mDeltaTime = timeData->mDeltaTime > 1.0 ? 1.0 : timeData->mDeltaTime;
        timeData->mTimeInSeconds += timeData->mDeltaTime;
        timeData->mLastTime = currentTime;
    }

    void Init()
    {
        globalTimeData.mStartTime = glfwGetTime();
        globalTimeData.mLastTime = glfwGetTime();
    }

    void Update()
    {
        updateTimeData(&globalTimeData);
    }

    double TimeInSeconds()
    {
        return globalTimeData.mTimeInSeconds;
    }

    double DeltaTime()
    {
        return globalTimeData.mDeltaTime;
    }

    double RealTimeInSeconds()
    {
        return glfwGetTime() - globalTimeData.mStartTime;
    }

    float GetTimeSpeed()
    {
        return globalTimeData.mTimeSpeed;
    }

    void SetTimeSpeed(float value)
    {
        globalTimeData.mTimeSpeed = value;
    }
}

}


