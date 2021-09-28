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

    template<typename T = std::chrono::seconds>
    auto GetElapsedTime() const;

    template<typename T = std::chrono::seconds>
    auto GetStartTime() const;

    template<typename T = std::chrono::seconds>
    auto GetStopTime() const;


};



//--------------------------------------------------------------------------------------
template<typename T>
inline auto Timer::GetElapsedTime() const
{
    T elapsed_time;

    if (m_hasStoped)
        elapsed_time = std::chrono::duration_cast<T>(m_stopTime - m_startTime);
    else
        elapsed_time = std::chrono::duration_cast<T>(std::chrono::steady_clock::now() - m_startTime);


    return elapsed_time.count();
}

template<typename T>
inline auto Timer::GetStartTime() const
{
    return std::chrono::time_point_cast<T>(m_startTime).time_since_epoch().count();
}

template<typename T>
inline auto Timer::GetStopTime() const
{
    return std::chrono::time_point_cast<T>(m_stopTime).time_since_epoch().count();
}
