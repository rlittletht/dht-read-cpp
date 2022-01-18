
#include <iostream>

#include "PiThread.h"
#include "timer.h"
#include <time.h>
#include <errno.h>
#include <sched.h>
#include <cstring>

using namespace Pi2;

namespace Pi2::Thread
{
/*------------------------------------------------------------------------------
  Function: PiClock::BusyWait

  consume waitDuration of time by spinning the CPU
  ------------------------------------------------------------------------------*/
pi_clock::duration BusyWait(pi_clock::duration waitDuration)
{
    Timer timer;
    
    while (timer.Elapsed() < waitDuration)
        ;

    return timer.Elapsed();
}

pi_clock::duration Sleep(pi_clock::duration waitDuration)
{
    Timer timer;

    // get nanoseconds to sleep
    struct timespec sleep;
    sleep.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(waitDuration).count();
    sleep.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(waitDuration).count()
        - sleep.tv_sec * 1000000000;

    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep, &sleep) && errno == EINTR)
        ;

    return timer.Elapsed();
}

SchedulerMaxPriorityBlock::SchedulerMaxPriorityBlock()
{
    struct sched_param sched;

    memset(&sched, 0, sizeof(sched));
    
    // Use FIFO scheduler with highest priority for the lowest chance of the kernel context switching.
    sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO, &sched);
}

SchedulerMaxPriorityBlock::~SchedulerMaxPriorityBlock()
{
    struct sched_param sched;
    
    memset(&sched, 0, sizeof(sched));
    
    // Go back to default scheduler with default 0 priority.
    sched.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &sched);
}

}
