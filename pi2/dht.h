
enum class SensorResult: int
{
    Success = 0,
    TimeoutError = 1,
    ChecksumError = 2,
    ArgumentError = 3,
    GpioError = 4
};

struct DhtReading
{
    float humidity;
    float temperature;
};

enum class SensorType: int
{
    AM2302,
    DHT22,
    DHT11
};

class Pi2Dht
{
public:
    static SensorResult ReadSensor(SensorType type, int pin, DhtReading &reading);
};

