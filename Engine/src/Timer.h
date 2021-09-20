#pragma once
#include "EnginePCH.h"

class Timer
{
public:
    Timer();                                                  
    Timer(const Timer& other) = delete;                       
    Timer(Timer&& other) = delete;                           
    Timer& operator=(const Timer& other) = delete;            
    Timer& operator=(Timer&& other) = delete;                 
    virtual ~Timer() {};

    void start();
    void stop();
    double getElapsedTime() const;  

private:
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> stopTime;
    bool hasStoped;
};

