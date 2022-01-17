
#pragma once

#include "PiThread.h"

class PiTimer
{
public:
    PiTimer();
    pi_clock::duration Elapsed();
    
private:
    pi_clock::time_point m_startTime;
};

    
