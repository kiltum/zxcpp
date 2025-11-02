#include "kempston.hpp"
#include <iostream>

// Constructor
Kempston::Kempston()
{
    // Initialize joystick state (all buttons released)
    joystickState = 0x00;
}

// Destructor
Kempston::~Kempston()
{
    // Nothing to clean up
}

// Read a byte from the specified port
uint8_t Kempston::readPort(uint16_t port)
{
    // Kempston joystick handles port 0x1F for joystick input
    if ((port & 0xFF) == 0x1F)
    {
        // Return the current joystick state
        // Bits 0-4: Right, Left, Down, Up, Fire
        // Bits 5-7: Reserved (should be 0)
        return joystickState & 0x1F;
    }

    return 0;
}

// Set the right direction button state
void Kempston::setRight(bool pressed)
{
    if (pressed)
    {
        joystickState |= 0x01; // Set bit 0
    }
    else
    {
        joystickState &= ~0x01; // Clear bit 0
    }
}

// Set the left direction button state
void Kempston::setLeft(bool pressed)
{
    if (pressed)
    {
        joystickState |= 0x02; // Set bit 1
    }
    else
    {
        joystickState &= ~0x02; // Clear bit 1
    }
}

// Set the down direction button state
void Kempston::setDown(bool pressed)
{
    if (pressed)
    {
        joystickState |= 0x04; // Set bit 2
    }
    else
    {
        joystickState &= ~0x04; // Clear bit 2
    }
}

// Set the up direction button state
void Kempston::setUp(bool pressed)
{
    if (pressed)
    {
        joystickState |= 0x08; // Set bit 3
    }
    else
    {
        joystickState &= ~0x08; // Clear bit 3
    }
}

// Set the fire button state
void Kempston::setFire(bool pressed)
{
    if (pressed)
    {
        joystickState |= 0x10; // Set bit 4
    }
    else
    {
        joystickState &= ~0x10; // Clear bit 4
    }
}

// Reset joystick state
void Kempston::reset()
{
    joystickState = 0x00; // All buttons released
}
