
#include "mmio.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace Pi2::MMIO
{
/*------------------------------------------------------------------------------
    Function: GPIO_Instance

    Return the singleton instance for the GPIO. You should call ConnectGPIO
    at least once to check the result.

    (This is not strictly necessary -- you could get the instance and check
    the status code yourself which is the same thing)
------------------------------------------------------------------------------*/
GPIO *GPIO_Instance()
{
    static GPIO s_gpio;

    return &s_gpio;
}

Result GPIO::StatusCode()
{
    return m_statusResult;
}

/*------------------------------------------------------------------------------
  Function: ConnectGPIO

  The connection implicitly happens when the object is created, and the
  object is implicitly created when we get the instance (on demand). So,
  just get the instance and return the StatusCode

  
 ------------------------------------------------------------------------------*/
Result ConnectGPIO()
{
    return GPIO_Instance()->StatusCode();
}

#define GPIO_BASE_OFFSET 0x200000
#define GPIO_LENGTH 4096

/*------------------------------------------------------------------------------
    Function: GPIO::GPIO

    This is RAII - when you acquire the resource, its initialized (and the
    memory map is connected). Check GPIO::StatusCode for result of init.
------------------------------------------------------------------------------*/
GPIO::GPIO()
{
    // Check for GPIO and peripheral addresses from device tree.
    // Adapted from code in the RPi.GPIO library at:
    //   http://sourceforge.net/p/raspberry-gpio-python/
    FILE *fp = fopen("/proc/device-tree/soc/ranges", "rb");

    if (fp == NULL)
    {
        m_statusResult = Result::Error_Offset;
        return;
    }
    fseek(fp, 4, SEEK_SET);
    unsigned char buf[4];
    if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf))
    {
        m_statusResult = Result::Error_Offset;
        return;
    }
    
    uint32_t peri_base = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
    uint32_t gpio_base = peri_base + GPIO_BASE_OFFSET;
    fclose(fp);

    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd == -1)
    {
        // Error opening /dev/gpiomem.
        m_statusResult = Result::Error_DevMem;
        return;
    }
    // Map GPIO memory to location in process space.
    m_gpioMemoryMap = (uint32_t*)mmap(NULL, GPIO_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_base);
    close(fd);
    
    if (m_gpioMemoryMap == MAP_FAILED)
    {
        // Don't save the result if the memory mapping failed.
        m_gpioMemoryMap = nullptr;
        m_statusResult = Result::Error_MMAP;
        return;
    }
    m_statusResult = Result::Success;
}

/*------------------------------------------------------------------------------
    Function: GPIO::SetInput

    Set GPIO register to 000 for specified GPIO number.
------------------------------------------------------------------------------*/
void GPIO::SetInput(const int gpio_number)
{
    *(m_gpioMemoryMap + ((gpio_number) / 10)) &= ~(7 << (((gpio_number) % 10) * 3));
}

void GPIO::SetOutput(const int gpio_number)
{
    // First set to 000 using input function.
    SetInput(gpio_number);
    // Next set bit 0 to 1 to set output.
    *(m_gpioMemoryMap + ((gpio_number) / 10)) |=  (1 << (((gpio_number) % 10) * 3));
}

void GPIO::SetHigh(const int gpio_number)
{
    *(m_gpioMemoryMap + 7) = 1 << gpio_number;
}
void GPIO::SetLow(const int gpio_number)
{
    *(m_gpioMemoryMap + 10) = 1 << gpio_number;
}

uint32_t GPIO::GetInput(const int gpio_number)
{
    return *(m_gpioMemoryMap + 13) & (1 << gpio_number);
}
}
