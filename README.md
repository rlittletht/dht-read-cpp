# dht-read-cpp
C++ code for reading a DH22/AM2302 sensor on a raspberry pi2 (and possible pi1)

Portions of this repo are subject to license from Adafruit, as provided by the license text at the top of those source files. These files originally from https://github.com/adafruit/Adafruit_Python_DHT.

Other portions of this code based on code from https://github.com/yamasy/pi_dht_read (which had no license designation).

All other portions of this code are subject to the MIT license.

# pi_dht_read (yasmasy)

This is Adafruit DHT11/22/AM2302 humidity/temperature sensor C/C++ library for Raspberry Pi.

This is derived from [Adafruit's driver](https://github.com/adafruit/Adafruit_Python_DHT)
with the changes to eliminate misreading data due to lack of real-timeness of Linux:
- Use BCM2708 1MHz counter instead of loop count in measuring pulse widths.
- Adjust measured pulse width by detecting the interrupts during the measurement.

