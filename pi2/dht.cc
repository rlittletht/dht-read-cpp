
#include <stdbool.h>
#include <stdlib.h>
#include "stdio.h"

#include "pi_2_dht_read.h"
#include "pi_2_mmio.h"
#include "../realtime.h"

#include "dht.h"
#include "../thread/PiClock.h"

// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
#define DHT_MAXCOUNT 32000

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES 41


SensorResult Pi2Dht::ReadSensor(SensorType type, int pin, DhtReading &reading)
{
    reading.temperature = 0.0f;
    reading.humidity = 0.0f;
    
    // Initialize GPIO library.
    if (pi_2_mmio_init() < 0)
	return SensorResult::GpioError;

    // Store the count that each DHT bit pulse is low and high.
    // Make sure array is initialized to start at zero.
    int pulseCounts[DHT_PULSES*2] = {0};
    
    // Set pin to output.
    pi_2_mmio_set_output(pin);
    
    // Bump up process priority and change scheduler to try to try to make process more 'real time'.
    set_max_priority();

    // Set pin high for ~500 milliseconds.
    pi_2_mmio_set_high(pin);
    sleep_milliseconds(500);

    // The next calls are timing critical and care should be taken
    // to ensure no unnecssary work is done below.

    // Set pin low for ~20 milliseconds.
    pi_2_mmio_set_low(pin);
    
    PiClock::BusyWait(std::chrono::steady_clock::duration(20ms));
//    busy_wait_milliseconds(20);

    // Set pin at input.
    pi_2_mmio_set_input(pin);
    // Need a very short delay before reading pins or else value is sometimes still low.
//    for (volatile int i = 0; i < 50; ++i)
//	;
    sleep_nanoseconds(100);

    // Wait for DHT to pull pin low.
    uint32_t count = 0;
    while (pi_2_mmio_input(pin))
    {
	if (++count >= DHT_MAXCOUNT)
	{
	    // Timeout waiting for response.
	    set_default_priority();
	    return SensorResult::TimeoutError;
	}
    }

    // Record pulse widths for the expected result bits.
    for (int i=0; i < DHT_PULSES*2; i+=2)
    {
	// Count how long pin is low and store in pulseCounts[i]
	while (!pi_2_mmio_input(pin))
	{
	    if (++pulseCounts[i] >= DHT_MAXCOUNT)
	    {
		// Timeout waiting for response.
		set_default_priority();
		return SensorResult::TimeoutError;
	    }
	}
	
	// Count how long pin is high and store in pulseCounts[i+1]
	while (pi_2_mmio_input(pin))
	{
//	sleep_milliseconds(1);
	    if (++pulseCounts[i+1] >= DHT_MAXCOUNT)
	    {
		// Timeout waiting for response.
		set_default_priority();
		return SensorResult::TimeoutError;
	    }
	}
//	printf("pulseCounts: %d\n", pulseCounts[i+1]);
    }

    // Done with timing critical code, now interpret the results.

    // Drop back to normal priority.
    set_default_priority();

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
	if (type == SensorType::DHT11)
	{
	    // Get humidity and temp for DHT11 sensor.
	    reading.humidity = (float)data[0];
	    reading.temperature = (float)data[2];
	}
	else if (type == SensorType::DHT22 || type == SensorType::AM2302)
	{
	    // Calculate humidity and temp for DHT22 sensor.
	    reading.humidity = (data[0] * 256 + data[1]) / 10.0f;
	    reading.temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
	    if (data[2] & 0x80)
		reading.temperature *= -1.0f;
	}
	return SensorResult::Success;
    }
    else
    {
	return SensorResult::ChecksumError;
    }
}
