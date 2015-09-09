/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCETimes.cpp ********/
/**************************************/

#include "../headers/SCETime.hpp"
#include "../headers/SCETools.hpp"
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
    Debug::Assert(!s_instance, "An instance of the Time system already exists");
    s_instance = new SCETime;
}

void SCETime::CleanUp()
{
    Debug::Assert(s_instance, "No Time system instance found, Init the system before using it");
    delete(s_instance);
    s_instance = nullptr;
}

void SCETime::Update()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    s_instance->update();
}

void SCETime::update()
{
    double currentTime = glfwGetTime();
    mDeltaTime = currentTime - mLastTime;
    mDeltaTime *= mTimeSpeed;
    mTimeInSeconds += mDeltaTime;
    mRealTimeInSeconds = mStartTime - currentTime;
    mLastTime = currentTime;
}

double SCETime::TimeInSeconds()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mTimeInSeconds;
}

double SCETime::DeltaTime()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mDeltaTime;
}

double SCETime::RealTimeInSeconds()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mRealTimeInSeconds;
}

float SCETime::GetTimeSpeed()
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    return s_instance->mTimeSpeed;
}

void SCETime::SetTimeSpeed(float value)
{
    Debug::Assert(s_instance, "No Lighting system instance found, Init the system before using it");
    s_instance->mTimeSpeed = value;
}

