/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETimes.cpp ********/
/**************************************/

#include "../headers/SCETime.hpp"
#include <glfw3.h>

using namespace SCE;
using namespace std;

SCETime* SCETime::s_instance;

SCETime::SCETime()
    : mTimeSpeed(1.0f),
      mTimeInSeconds(0.0),
      mDeltaTime(0.0),
      mRealTimeInSeconds(0.0)
{
    mStartTime = glfwGetTime();
    mLastTime = glfwGetTime();
    s_instance = this;
}

SCETime::~SCETime()
{
}

void SCETime::Init()
{
    s_instance = new SCETime;
}

void SCETime::CleanUp()
{
    delete(s_instance);
    s_instance = nullptr;
}

void SCETime::Update()
{
    s_instance->update();
}

void SCETime::update()
{
    /*clock_t currClock = clock();
    SCE_DEBUG_LOG("clock : %d", (int)currClock);
    double deltaTicks = currClock - last_clock;
    s_deltaTime = deltaTicks / CLOCKS_PER_SEC;
    s_deltaTime *= timeSpeed;
    last_clock = currClock;
    double ticks = currClock - start_clock;
    s_realTimeInSeconds = ticks / CLOCKS_PER_SEC;
    s_timeInSeconds += s_deltaTime;*/

    double currentTime = glfwGetTime();
    mDeltaTime = currentTime - mLastTime;
    mDeltaTime *= mTimeSpeed;
    mTimeInSeconds += mDeltaTime;
    mRealTimeInSeconds = mStartTime - currentTime;
    mLastTime = currentTime;
}

double SCETime::TimeInSeconds()
{
    return s_instance->mTimeInSeconds;
}

double SCETime::DeltaTime()
{
    return s_instance->mDeltaTime;
}

double SCETime::RealTimeInSeconds()
{
    return s_instance->mRealTimeInSeconds;
}

float SCETime::GetTimeSpeed()
{
    return s_instance->mTimeSpeed;
}

void SCETime::SetTimeSpeed(float value)
{
    s_instance->mTimeSpeed = value;
}

