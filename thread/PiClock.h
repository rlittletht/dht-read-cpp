
#include <chrono>
using namespace std::chrono_literals;

class PiClock
{
public:
    static std::chrono::steady_clock::duration BusyWait(std::chrono::steady_clock::duration waitDuration);
};

    
