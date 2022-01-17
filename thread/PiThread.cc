
#include "PiThread.h"
#include "timer.h"

namespace PiThread
{
/*------------------------------------------------------------------------------
  Function: PiClock::BusyWait

  consume waitDuration of time by spinning the CPU
 ------------------------------------------------------------------------------*/
pi_clock::duration BusyWait(pi_clock::duration waitDuration)
{
    PiTimer timer;
    
    while (timer.Elapsed() < waitDuration)
        ;

    return timer.Elapsed();
}

}
