// -*- mode: c++ -*-

#pragma once

#include <chrono>

namespace PiThread
{
using pi_clock = std::chrono::steady_clock;

pi_clock::duration BusyWait(pi_clock::duration waitDuration);
pi_clock::duration Sleep(pi_clock::duration waitDuration);

/*------------------------------------------------------------------------------
    Class: ScheduleMaxPriorityBlock

    This RAII class will change the scheduler to use a FIFO scheduler with
    highest priority for the lowest chance of the kernel context switching.

    When this object passes out of scope, it will restore the scheduler
    to defaults
------------------------------------------------------------------------------*/
class SchedulerMaxPriorityBlock
{
public:
    SchedulerMaxPriorityBlock();
    ~SchedulerMaxPriorityBlock();
};

}

using namespace std::chrono_literals;
