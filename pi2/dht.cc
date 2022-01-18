

#include <iostream>
#include <iomanip>

#include "mmio.h"
#include "dht.h"
#include "../thread/PiThread.h"
#include "../thread/timer.h"
#include <algorithm>
#include <cmath>

#define DHT_MIN_READINTERVAL 2s
#define DHT_TIMEOUT 525ms

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES 41

using namespace Pi2;
using namespace Dht;

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

Sensor::Sensor(Model model, int pin)
{
    m_model = model;
    m_pin = pin;
    m_lastReading = Pi2::Thread::pi_clock::now() - 1h;
}


/*------------------------------------------------------------------------------
    Function: Sensor::GetReading

    Get a reading from the sensor on the given pin. This will take around
    525msec. This will also enforce the sensor's recommended 2s interval
    between reading.
------------------------------------------------------------------------------*/
Result Sensor::GetReading(Reading &reading)
{
    Pi2::Thread::pi_clock::duration sinceLastReading = Pi2::Thread::pi_clock::now() - m_lastReading;
    
    if (sinceLastReading < Pi2::Thread::pi_clock::duration(DHT_MIN_READINTERVAL))
        Pi2::Thread::pi_clock::duration dur = Pi2::Thread::Sleep(DHT_MIN_READINTERVAL - sinceLastReading);

    m_lastReading = Pi2::Thread::pi_clock::now();
    
    reading.temperature = 0.0f;
    reading.humidity = 0.0f;

    // Initialize GPIO library.
    if (Pi2::MMIO::ConnectGPIO() != Pi2::MMIO::Result::Success)
	return Result::GpioError;

    // Store the count that each DHT bit pulse is low and high.
    // this is the count of microseconds per pulse
    int usPulseWidths[DHT_PULSES * 2] = { 0 };
    
    Pi2::MMIO::GPIO *gpio = Pi2::MMIO::GPIO_Instance();
    // Set pin to output.
  
    gpio->SetOutput(m_pin);
    Pi2::Timer timer; // create the timer outside the max priority block

    // BLOCK for MaxPriority
    {
        // Bump up process priority and change scheduler to try to try to make process more 'real time'.
        Pi2::Thread::SchedulerMaxPriorityBlock maxPriorityBlock;

        // Set pin high for ~500 milliseconds.
        gpio->SetHigh(m_pin);
        Pi2::Thread::Sleep(Pi2::Thread::pi_clock::duration(500ms));

        // The next calls are timing critical and care should be taken
        // to ensure no unnecssary work is done below.

        // Set pin low for ~20 milliseconds.
        gpio->SetLow(m_pin);
    
        Pi2::Thread::BusyWait(std::chrono::steady_clock::duration(20ms));

        // Set pin at input.
        gpio->SetInput(m_pin);
    
        // Need a very short delay before reading pins or else value is sometimes still low.
        Pi2::Thread::Sleep(Pi2::Thread::pi_clock::duration(50ns));
        
        // Wait for DHT to pull pin low.
        timer.Reset();
        while (gpio->GetInput(m_pin))
        {
            if (timer.Elapsed() > Pi2::Thread::pi_clock::duration(DHT_TIMEOUT))
                return Result::TimeoutError;
        }

        // Record pulse widths for the expected result bits.
        for (int i = 0; i < DHT_PULSES*2; i += 2)
        {
            timer.Reset();
            // Count how long pin is low and store in pulseCounts[i]
            while (!gpio->GetInput(m_pin))
            {
                if (timer.Elapsed() > Pi2::Thread::pi_clock::duration(DHT_TIMEOUT))
                    return Result::TimeoutError;
            }
            usPulseWidths[i] = std::chrono::duration_cast<std::chrono::microseconds>(timer.Elapsed()).count();

            timer.Reset();
            // Count how long pin is high and store in pulseCounts[i+1]
            while (gpio->GetInput(m_pin))
            {
                if (timer.Elapsed() > Pi2::Thread::pi_clock::duration(DHT_TIMEOUT))
                    return Result::TimeoutError;
            }
            usPulseWidths[i + 1] = std::chrono::duration_cast<std::chrono::microseconds>(timer.Elapsed()).count();
        }

        // Done with timing critical code, now interpret the results.
        // Drop back to normal priority (happens automatically when we leave this block)
    } // BLOCK for Max Priority
    
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
	if (m_model == Model::DHT11)
	{
	    // Get humidity and temp for DHT11 sensor.
	    reading.humidity = (float)data[0];
	    reading.temperature = (float)data[2];
	}
	else if (m_model == Model::DHT22 || m_model == Model::AM2302)
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
