
namespace Pi2Dht
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
    static Result GetReading(Model model, int pin, Reading &reading);
};

}
