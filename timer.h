#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <mutex>
#include <string>

/**
 * @class Timer
 * 
 * @brief Timer class (for reference).
 * 
 * Mainly used as time reference for different functions.
 * Constructor registers start time, destructor calculates
 * the time elapsed and prints it.
 * 
 */

class Timer {

    public:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
        std::string m_name;
        double m_nanos;
    
        Timer(const std::string& name);
        ~Timer();
};

#endif // TIMER_H