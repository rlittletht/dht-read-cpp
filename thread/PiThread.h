// -*- mode: c++ -*-

#pragma once

#include <chrono>

namespace PiThread
{
using pi_clock = std::chrono::steady_clock;

pi_clock::duration BusyWait(pi_clock::duration waitDuration);
}

using namespace std::chrono_literals;
