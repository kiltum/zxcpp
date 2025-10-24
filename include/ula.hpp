#ifndef ULA_HPP
#define ULA_HPP

#include <cstdint>
#include <memory>
#include "memory.hpp"
#include "port.hpp"

class ULA {
private:
    Memory* memory;
    
    // Screen buffer (256x192 pixels with border)
    uint32_t* screenBuffer;
    
    // Pre-calculated color values for faster lookup
    uint32_t colors[16];
    
    // ULA internal state
    
    int line;
    bool flash;
    int flashCnt;
    int frameCnt;
    int horClock; 
    
    // Border color
    uint8_t borderColor;
    
    // Keyboard state (8 half-rows of 5 keys each)
    // 0 = pressed, 1 = released (inverted logic)
    uint8_t keyboard[8];
    
    // Private helper functions
    void drawPixel(int);
    uint32_t getPixelColorFast(uint8_t x, uint8_t y);
    bool audioState; // Audio output state

public:
uint32_t clock;
    // Constructor
    ULA(Memory* mem);
    
    // Destructor
    ~ULA();
    
    // Port handling functions
    uint8_t readPort(uint16_t port);
    void writePort(uint16_t port, uint8_t value);
    
    // Get screen buffer
    uint32_t* getScreenBuffer();
    
    // Process a single ULA tick
    // Returns internal clock state. If 0 - screen is ready, generate interrupt
    int oneTick();
    
    // Reset ULA state
    void reset();
    
    // Keyboard handling functions
    void setKeyState(int halfRow, uint8_t keyMask);
    void setKeyDown(int halfRow, int keyBit);
    void setKeyUp(int halfRow, int keyBit);
};

#endif // ULA_HPP
