
#pragma once

#include "PiThread.h"

using namespace Pi2::Thread;

namespace Pi2
{
class Timer
{
public:
    Timer();
    pi_clock::duration Elapsed();
    void Reset();
    
private:
    pi_clock::time_point m_startTime;
};
}
