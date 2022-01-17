

#include <iostream>
#include "mmio.h"
#include "dht.h"
#include "../thread/PiThread.h"

// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
#define DHT_MAXCOUNT 32000

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES 41

using namespace Pi2Dht;

Result Sensor::GetReading(Model model, int pin, Reading &reading)
{
    reading.temperature = 0.0f;
    reading.humidity = 0.0f;

    // Initialize GPIO library.
    if (Pi2::MMIO::ConnectGPIO() != Pi2::MMIO::Result::Success)
	return Result::GpioError;

    // Store the count that each DHT bit pulse is low and high.
    // Make sure array is initialized to start at zero.
    int pulseCounts[DHT_PULSES*2] = {0};

    Pi2::MMIO::GPIO *gpio = Pi2::MMIO::GPIO_Instance();
    // Set pin to output.
  
    gpio->SetOutput(pin);

    // BLOCK for MaxPriority
    {
        // Bump up process priority and change scheduler to try to try to make process more 'real time'.
        PiThread::SchedulerMaxPriorityBlock maxPriorityBlock;

        // Set pin high for ~500 milliseconds.
        gpio->SetHigh(pin);
        PiThread::Sleep(PiThread::pi_clock::duration(500ms));

        // The next calls are timing critical and care should be taken
        // to ensure no unnecssary work is done below.

        // Set pin low for ~20 milliseconds.
        gpio->SetLow(pin);
    
        PiThread::BusyWait(std::chrono::steady_clock::duration(20ms));

        // Set pin at input.
        gpio->SetInput(pin);
    
        // Need a very short delay before reading pins or else value is sometimes still low.
        PiThread::Sleep(PiThread::pi_clock::duration(100ns));

        // Wait for DHT to pull pin low.
        uint32_t count = 0;
        while (gpio->GetInput(pin))
        {
            // Timeout waiting for response.
            if (++count >= DHT_MAXCOUNT)
                return Result::TimeoutError;
        }

        // Record pulse widths for the expected result bits.
        for (int i=0; i < DHT_PULSES*2; i+=2)
        {
            // Count how long pin is low and store in pulseCounts[i]
            while (!gpio->GetInput(pin))
            {
                // Timeout waiting for response.
                if (++pulseCounts[i] >= DHT_MAXCOUNT)
                    return Result::TimeoutError;
            }
	
            // Count how long pin is high and store in pulseCounts[i+1]
            while (gpio->GetInput(pin))
            {
//	sleep_milliseconds(1);
                // Timeout waiting for response.
                if (++pulseCounts[i+1] >= DHT_MAXCOUNT)
                    return Result::TimeoutError;
            }
//	printf("pulseCounts: %d\n", pulseCounts[i+1]);
        }

        // Done with timing critical code, now interpret the results.
        // Drop back to normal priority (happens automatically when we leave this block)
    } // BLOCK for Max Priority
    
    // Compute the average low pulse width to use as a 50 microsecond reference threshold.
    // Ignore the first two readings because they are a constant 80 microsecond pulse.
    uint32_t threshold = 0;
    for (int i=2; i < DHT_PULSES*2; i+=2)
	threshold += (uint32_t)pulseCounts[i];
    
    threshold /= DHT_PULSES-1;

    // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
    // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
    // then it must be a ~70us 1 pulse.
    uint8_t data[5] = {0};
    for (int i=3; i < DHT_PULSES*2; i+=2)
    {
	int index = (i-3)/16;
	data[index] <<= 1;
	if ((uint32_t)pulseCounts[i] >= threshold)
	{
	    // One bit for long pulse.
	    data[index] |= 1;
	}
	// Else zero bit for short pulse.
    }

    // Useful debug info:
    //printf("Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n", data[0], data[1], data[2], data[3], data[4]);
    
    // Verify checksum of received data.
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
    {
	if (model == Model::DHT11)
	{
	    // Get humidity and temp for DHT11 sensor.
	    reading.humidity = (float)data[0];
	    reading.temperature = (float)data[2];
	}
	else if (model == Model::DHT22 || model == Model::AM2302)
	{
	    // Calculate humidity and temp for DHT22 sensor.
	    reading.humidity = (data[0] * 256 + data[1]) / 10.0f;
	    reading.temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
	    if (data[2] & 0x80)
		reading.temperature *= -1.0f;
	}
	return Result::Success;
    }
    else
    {
	return Result::ChecksumError;
    }
}
