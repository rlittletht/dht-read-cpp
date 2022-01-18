
#include "timer.h"

namespace Pi2
{
Timer::Timer()
{
    m_startTime = pi_clock::now();
}

pi_clock::duration Timer::Elapsed()
{
    return pi_clock::now() - m_startTime;
}

void Timer::Reset()
{
    m_startTime = pi_clock::now();
}
}
