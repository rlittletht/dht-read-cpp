#include <stdio.h>
#include <stdlib.h>
#include "pi/pi_dht_read.h"
#include "pi2/pi_2_dht_read.h"

#include "pi2/dht.h"

#include "thread/timer.h"

#include <iostream>

// GPIO pin number for DHT sensor
#define DHTPIN 4

#include <string>

int main(int argc, const char **argv)
{
    float humidity, temperature;
    bool useNewReader = false;
    bool testMode = false;
    int iArg = 1;
    int readingsAttempted = 0;
    int successfulReadings = 0;
    PiTimer timer;
    
    while (iArg < argc)
    {
        std::string arg(argv[iArg]);

        if (arg == "new")
        {
            useNewReader = true;
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
            Pi2Dht::Reading reading;
            Pi2Dht::Result result = Pi2Dht::Sensor::GetReading(Pi2Dht::Model::AM2302, DHTPIN, reading);

            if (result == Pi2Dht::Result::Success)
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

//    auto a = timer.Elapsed();
//    std::chrono::milliseconds msec(std::chrono::duration_cast<std::chrono::milliseconds>(a));
    
    std::cout << "Total elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timer.Elapsed()).count() << " msec\n";
    return 0;
}
