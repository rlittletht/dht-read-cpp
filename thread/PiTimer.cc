
#include "PiTimer.h"

PiTimer::PiTimer()
{
    m_startTime = pi_clock::now();
}

pi_clock::duration PiTimer::Elapsed()
{
    return pi_clock::now() - m_startTime;
}
