#include <stdio.h>
#include <stdlib.h>
#include "pi2/pi_2_dht_read.h"

#include "pi2/dht.h"

#include "thread/timer.h"
#include <mutex>
#include <vector>

#include <iostream>
#include <csignal>
#include <thread>

// GPIO pin number for DHT sensor
#define DHTPIN 25
//4

#include <string>

std::vector<Pi2::Dht::Reading> readings;
bool fReadingsAvailable = false;
std::mutex readingsGuard;
std::shared_ptr<std::mutex> spMutexScheduler = std::make_shared<std::mutex>();
int cSensorsReading = 0;
bool fAbortReading = false;

void PushReading(const Pi2::Dht::Reading &reading)
{
    std::scoped_lock<std::mutex> lock(readingsGuard);

    readings.push_back(std::move(reading));
    fReadingsAvailable = true;
}

void ReadSensorInLoop(int pin)
{
    Pi2::Dht::Sensor sensor(spMutexScheduler, Pi2::Dht::Model::AM2302, pin);

    cSensorsReading++;
    // guarantee we get at least one reading
    do
    {
        Pi2::Dht::Reading reading;
        
        if (sensor.GetReading(reading) == Pi2::Dht::Result::Success)
            PushReading(reading);
    } while (!fAbortReading);
    
    cSensorsReading--;
}

void ReportPendingReadings()
{
    std::scoped_lock<std::mutex> lock(readingsGuard);

    for (Pi2::Dht::Reading reading : readings)
    {
        std::cout << "[" << reading.pinAddress << "] Reading: temp="
                  << reading.temperature
                  << ", humidity = "
                  << reading.humidity
                  << "\n";
    }

    readings.clear();
    fReadingsAvailable = false;
}

void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        if (fAbortReading)
        {
            std::cout << "Received SIGINT again...terminating...\n";
            exit(signum);
        }

        std::cout << "Received SIGINT...requesting abort...\n";
        fAbortReading = true;
        
        while (cSensorsReading > 0)
            Pi2::Thread::Sleep(Pi2::Thread::pi_clock::duration(50ms));

        ReportPendingReadings();
    }

    exit(signum);
}
    
void DoThreadedReads(int pin1, int pin2)
{
    signal(SIGINT, signalHandler);
    
//    fAbortReading = true;
    std::thread work1(ReadSensorInLoop, pin1);
    std::thread work2(ReadSensorInLoop, pin2);

    do
    {
        if (fReadingsAvailable)
            ReportPendingReadings();

        // sleep this thread while we wait for other threads to read
        Pi2::Thread::Sleep(Pi2::Thread::pi_clock::duration(500ms));
    } while (!fAbortReading);
}


int main(int argc, const char **argv)
{
    float humidity, temperature;
    bool useNewReader = false;
    bool testMode = false;
    bool multithreaded = false;
    int iArg = 1;
    int readingsAttempted = 0;
    int successfulReadings = 0;
    Pi2::Timer timer;
    
    while (iArg < argc)
    {
        std::string arg(argv[iArg]);

        if (arg == "new")
        {
            useNewReader = true;
            iArg++;
            continue;
        }

        if (arg == "mt")
        {
            multithreaded = true;
            iArg++;
            continue;
        }
        
        if (arg == "test")
        {
            testMode = true;
            iArg++;
            continue;
        }
        // not recognized. break
        break;
    }

    int count = argc < (iArg + 1) ? 1 : atoi(argv[iArg]);

    if (multithreaded)
    {
        DoThreadedReads(4, 25);
        return 0;
    }
    else
    {
        Pi2::Dht::Sensor sensor(spMutexScheduler, Pi2::Dht::Model::AM2302, DHTPIN);

        if (testMode)
        {
            count = 10;
            printf("testing");
        }
    
        while (count-- > 0)
        {
            readingsAttempted++;
        
            if (useNewReader)
            {
                Pi2::Dht::Reading reading;
                Pi2::Dht::Result result = sensor.GetReading(reading);

                if (result == Pi2::Dht::Result::Success)
                {
                    if (!testMode)
                        printf("new: temperature:%.1f Humidity:%.1f\n", reading.temperature, reading.humidity);
                    else
                        printf(".");
                
                    successfulReadings++;
                }
                else
                {
                    if (!testMode)
                        printf("read failed: %d\n", (int)result);
                }
            }
            else
            {
                int success = pi_2_dht_read(_AM2302, DHTPIN, &humidity, &temperature);

                if (success == DHT_SUCCESS)
                {
                    if (!testMode)
                        printf("temperature:%.1f Humidity:%.1f\n", temperature, humidity);
                    else
                        printf(".");
                    successfulReadings++;
                }
                else
                {
                    if (!testMode)
                        printf("read failed: %d\n", success);
                }
            }
            if (count > 0 && !testMode)
            {
                printf("Press ENTER key to continue\n");
                getchar();
            }
        }

        if (testMode)
        {
            printf("\n\nRESULTS: %d/%d.  %f%%\n", successfulReadings, readingsAttempted, (float)(successfulReadings * 100.0) / (float)readingsAttempted);
        }

        std::cout << "Total elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timer.Elapsed()).count() << " msec\n";
        return 0;
    }
}
