
#pragma once

#include "PiThread.h"

namespace PiThread
{
class PiClock
{
public:
    static pi_clock::duration BusyWait(pi_clock::duration waitDuration);
};
}



    
