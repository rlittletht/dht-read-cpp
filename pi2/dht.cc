

#include <iostream>
#include <iomanip>

#include "mmio.h"
#include "dht.h"
#include "../thread/PiThread.h"
#include "../thread/timer.h"
#include <algorithm>
#include <cmath>

// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
#define DHT_MAXCOUNT 32000
#define DHT_TIMEOUT 2s

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES 41

using namespace Pi2Dht;

void reportThresholds(
    int usPulseWidths[DHT_PULSES * 2],
    int pulseCounts[DHT_PULSES * 2])
{
    int threshold = 0;

    // figure out how to convert pulse counts to usecs:

    for(int i = 1; i < DHT_PULSES; i++)
    {
        threshold += pulseCounts[i * 2];
    }

    threshold /= DHT_PULSES;
    threshold /= 50;
    
    std::cout << "Pulse Width Comparisons\n\n";
    std::cout << std::setw(10) << "us(C)0"
              << std::setw(10) << "us(C)1"
              << std::setw(10) << "us(T)0"
              << std::setw(10) << "us(T)1\n";

    for (int i = 0; i < DHT_PULSES * 2; i += 2)
    {
        std::cout << std::setw(10) << pulseCounts[i] / threshold
                  << std::setw(10) << pulseCounts[i + 1] / threshold
                  << std::setw(10) << usPulseWidths[i]
                  << std::setw(10) << usPulseWidths[i + 1]
                  << "\n";
    }
}

/*------------------------------------------------------------------------------
    Function: GetBitFromSignal

    Get the bit value (0 or 1) from the signal.

    A signal is a pair of pulse widths (the 50us reference low signal, and the
    data high signal)
------------------------------------------------------------------------------*/
inline int GetBitFromSignal(int signal[2])
{
    return signal[1] > signal[0];
}

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

    // this is the count of microseconds per pulse
    int usPulseWidths[DHT_PULSES * 2] = { 0 };
    
    Pi2::MMIO::GPIO *gpio = Pi2::MMIO::GPIO_Instance();
    // Set pin to output.
  
    gpio->SetOutput(pin);
    PiTimer timer; // create the timer outside the max priority block

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
        PiThread::Sleep(PiThread::pi_clock::duration(50ns));

        // Wait for DHT to pull pin low.
//        uint32_t count = 0;
        
        timer.Reset();
        while (gpio->GetInput(pin))
        {
            if (timer.Elapsed() > PiThread::pi_clock::duration(DHT_TIMEOUT))
                return Result::TimeoutError;
        }

        // Record pulse widths for the expected result bits.
        for (int i = 0; i < DHT_PULSES*2; i += 2)
        {
            timer.Reset();
            // Count how long pin is low and store in pulseCounts[i]
            while (!gpio->GetInput(pin))
            {
                // Timeout waiting for response.
                if (++pulseCounts[i] >= DHT_MAXCOUNT)
                    return Result::TimeoutError;
            }
            usPulseWidths[i] = std::chrono::duration_cast<std::chrono::microseconds>(timer.Elapsed()).count();

            timer.Reset();
            // Count how long pin is high and store in pulseCounts[i+1]
            while (gpio->GetInput(pin))
            {
                // Timeout waiting for response.
                if (++pulseCounts[i+1] >= DHT_MAXCOUNT)
                    return Result::TimeoutError;
            }
            usPulseWidths[i + 1] = std::chrono::duration_cast<std::chrono::microseconds>(timer.Elapsed()).count();
        }

        // Done with timing critical code, now interpret the results.
        // Drop back to normal priority (happens automatically when we leave this block)
    } // BLOCK for Max Priority
    
    // Compute the average low pulse width to use as a 50 microsecond reference threshold.
    // Ignore the first two readings because they are a constant 80 microsecond pulse.
    uint32_t threshold = 0;
    for (int i=2; i < DHT_PULSES*2; i+=2)
	threshold += (uint32_t)pulseCounts[i];
    
    reportThresholds(usPulseWidths, pulseCounts);
    threshold /= DHT_PULSES-1;

    // Interpret each high pulse as a 0 or 1 by comparing it its 50us reference signal
    // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
    // then it must be a ~70us 1 pulse.
    uint8_t data[5] = {0};
    for (int i = 2; i < DHT_PULSES * 2; i += 2)
    {
	int index = (i - 2)/16;
	data[index] <<= 1;
        data[index] |= GetBitFromSignal(&usPulseWidths[i]);
    }

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
