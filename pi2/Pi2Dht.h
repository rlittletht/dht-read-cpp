
enum class SensorResult: int
{
    Success,
    TimeoutError,
    ChecksumError,
    ArgumentError,
    GpioError
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

