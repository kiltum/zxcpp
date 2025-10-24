#ifndef KEMPSTON_HPP
#define KEMPSTON_HPP

#include <cstdint>
#include "port.hpp"

class Kempston {
private:
    // Joystick state
    // Bit 0: Right
    // Bit 1: Left
    // Bit 2: Down
    // Bit 3: Up
    // Bit 4: Fire
    uint8_t joystickState;

public:
    // Constructor
    Kempston();
    
    // Destructor
    ~Kempston();
    
    // Port handling functions
    uint8_t readPort(uint16_t port);
    
    // Joystick state manipulation functions
    void setRight(bool pressed);
    void setLeft(bool pressed);
    void setDown(bool pressed);
    void setUp(bool pressed);
    void setFire(bool pressed);
    
    // Reset joystick state
    void reset();
};

#endif // KEMPSTON_HPP
