#ifndef AY8912_HPP
#define AY8912_HPP

#include <cstdint>

class AY8912
{
private:
    // AY-3-8912 internal state
    uint8_t registers[16];  // 14 registers (0-13), 14-15 unused
    uint8_t selectedRegister;
    bool addressLatch;

public:
    AY8912();
    ~AY8912();
    
    bool initialize();
    void cleanup();
    
    // Port handlers for ZX Spectrum 128
    void writePort(uint16_t port, uint8_t value);
    uint8_t readPort(uint16_t port);
    
    // Reset the chip
    void reset();
};

#endif // AY8912_HPP
