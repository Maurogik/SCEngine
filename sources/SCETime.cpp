/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETimes.cpp ********/
/**************************************/

#include "../headers/SCETime.hpp"
#include <glfw3.h>

using namespace SCE;
using namespace std;

double  SCETime::s_timeInSeconds = 0;
double  SCETime::s_realTimeInSeconds = 0;
double  SCETime::s_deltaTime = 0;
float   SCETime::s_timeSpeed = 1.0f;
double  SCETime::start_time = glfwGetTime();
double  SCETime::last_time = glfwGetTime();


const double &SCETime::TimeInSeconds()
{
    return s_timeInSeconds;
}

const double &SCETime::DeltaTime()
{
    return s_deltaTime;
}

const double &SCETime::RealTimeInSeconds()
{
    return s_realTimeInSeconds;
}

void SCETime::Update()
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
    s_deltaTime = currentTime - last_time;
    s_deltaTime *= s_timeSpeed;
    s_timeInSeconds += s_deltaTime;
    s_realTimeInSeconds = start_time - currentTime;
    last_time = currentTime;
}
const float& SCETime::TimeSpeed()
{
    return s_timeSpeed;
}

void SCETime::SetTimeSpeed(float value)
{
    s_timeSpeed = value;
}

