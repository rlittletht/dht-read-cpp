
#include "../thread/PiThread.h"
#include <memory>
#include <mutex>

namespace Pi2
{
namespace Dht
{

enum class Result: int
{
    Success = 0,
    TimeoutError = 1,
    ChecksumError = 2,
    ArgumentError = 3,
    GpioError = 4
};

struct Reading
{
    float humidity;
    float temperature;
    int pinAddress;
    Thread::pi_clock::time_point readingTime;
};

enum class Model: int
{
    AM2302,
    DHT22,
    DHT11
};

class Sensor
{
public:
    Sensor(std::shared_ptr<std::mutex> spMutexScheduler, Model model, int pin);
    Result GetReading(Reading &reading);

private:
    Model m_model;
    int m_pin;
    Thread::pi_clock::time_point m_lastReading;
    std::shared_ptr<std::mutex> m_spMutexScheduler;
};

}
}
