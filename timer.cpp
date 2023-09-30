#include <iostream>
#include <mutex>

#include "timer.h"

std::mutex printMtx;

Timer::Timer(const std::string& name)
    : m_start(std::chrono::high_resolution_clock::now()),
      m_name(name) {
        {
            std::unique_lock<std::mutex> lock(printMtx);
            std::cout << "Starting timer for " << m_name << std::endl;
        }
      }

Timer::~Timer() {
    m_end = std::chrono::high_resolution_clock::now();
    m_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(m_end - m_start).count();
    {
        std::unique_lock<std::mutex> lock(printMtx);
        std::cout << "Timer for " << m_name << ": " << (m_nanos / 1e6) << " ms" << std::endl;
    }
}