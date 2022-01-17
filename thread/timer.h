
#pragma once

#include "PiThread.h"

using namespace PiThread;

class PiTimer
{
public:
    PiTimer();
    pi_clock::duration Elapsed();
    void Reset();
    
private:
    pi_clock::time_point m_startTime;
};

    
