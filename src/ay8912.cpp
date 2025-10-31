#include "ay8912.hpp"
#include <iostream>
#include <cstring>

AY8912::AY8912() : selectedRegister(0), addressLatch(false)
{
    // Initialize registers
    std::memset(registers, 0, sizeof(registers));
}

AY8912::~AY8912()
{
    cleanup();
}

bool AY8912::initialize()
{
    // Empty stub - does nothing
    return true;
}

void AY8912::cleanup()
{
    // Empty stub - does nothing
}

void AY8912::reset()
{
    // Empty stub - does nothing
    std::memset(registers, 0, sizeof(registers));
    selectedRegister = 0;
    addressLatch = false;
}

void AY8912::writePort(uint16_t port, uint8_t value)
{
    if (port == 0xFFFD)
    {
        printf("AD %x %x\n",port,value); // Show what to write to port for debug
        // Write register number
        selectedRegister = value & 0x0F; // Only lower 4 bits are valid
        addressLatch = true;
    }
    
    else if (port == 0xBFFD) 
    {
        printf("AR %x %x\n",port,value); // Show what to write to port for debug
        // Write data to selected register
        if (addressLatch && selectedRegister <= 13)
        {
            registers[selectedRegister] = value;
            addressLatch = false; // Reset latch after writing data
        }
    }
}

uint8_t AY8912::readPort(uint16_t port)
{
    if (port == 0xFFFD)
    {
        if (addressLatch && selectedRegister <= 13)
        {
            // Read from selected register
            return registers[selectedRegister];
        }
        else
        {
            // Return 0 if no address is latched
            return 0;
        }
    }
    return 0;
}
