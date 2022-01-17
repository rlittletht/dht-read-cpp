// -*- mode: c++ -*-
#pragma once

#include <cstdint>

namespace Pi2::MMIO
{
enum class Result: int
{
    Success = 0,
    Error_DevMem = -1,
    Error_MMAP = -2,
    Error_Offset = -3,
    Error_Unknown = -4
};

class GPIO
{
public:
    GPIO();
    void SetInput(const int gpio_number);
    void SetOutput(const int gpio_number);
    void SetHigh(const int gpio_number);
    void SetLow(const int gpio_number);
    uint32_t GetInput(const int gpio_number);
    Result StatusCode();
    
private:
    volatile uint32_t *m_gpioMemoryMap = nullptr;
    Result m_statusResult = Result::Error_Unknown;
};

Result ConnectGPIO();
GPIO *GPIO_Instance();
}
