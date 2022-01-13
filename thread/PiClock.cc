
#include "PiClock.h"

/*------------------------------------------------------------------------------
    Function: PiClock::BusyWait

    consume waitDuration of time by spinning the CPU
------------------------------------------------------------------------------*/
std::chrono::steady_clock::duration PiClock::BusyWait(std::chrono::steady_clock::duration waitDuration)
{
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - startTime < waitDuration)
        ;
    
    return std::chrono::steady_clock::now() - startTime;
}
