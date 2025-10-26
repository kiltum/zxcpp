#include "tape.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

// Constructor
Tape::Tape()
{
    reset();
}

// Destructor
Tape::~Tape()
{
    // Nothing to clean up
}

// Reset tape state
void Tape::reset()
{
    isTapePlayed = false;
}

// Get next audio input state for ULA
bool Tape::getNextBit()
{
    return false;
}
