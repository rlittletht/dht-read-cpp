
#include "PiClock.h"
#include "timer.h"

/*------------------------------------------------------------------------------
    Function: PiClock::BusyWait

    consume waitDuration of time by spinning the CPU
------------------------------------------------------------------------------*/
pi_clock::duration PiClock::BusyWait(pi_clock::duration waitDuration)
{
    PiTimer timer;
    
    while (timer.Elapsed() < waitDuration)
        ;

    return timer.Elapsed();
}
