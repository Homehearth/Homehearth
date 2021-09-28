#pragma once
#include "EnginePCH.h"

class Timer
{
private:
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    std::chrono::time_point<std::chrono::steady_clock> m_stopTime;
    bool m_hasStoped;

	
public:
    Timer();                                                  
    Timer(const Timer& other) = delete;                       
    Timer(Timer&& other) = delete;                           
    Timer& operator=(const Timer& other) = delete;            
    Timer& operator=(Timer&& other) = delete;                 
    virtual ~Timer() {};

    void Start();
    void Stop();
    double GetElapsedTime() const;  

};

