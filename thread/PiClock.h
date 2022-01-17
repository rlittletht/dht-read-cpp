
#pragma once

#include "PiThread.h"

namespace PiThread
{

/*------------------------------------------------------------------------------
    Class: PiClock

------------------------------------------------------------------------------*/
class PiClock
{
public:
    static pi_clock::duration BusyWait(pi_clock::duration waitDuration);
};

}



    
