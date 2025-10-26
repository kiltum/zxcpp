#ifndef TAPE_HPP
#define TAPE_HPP

#include <cstdint>
#include <string>
#include <vector>

class Tape {
private:
    
public:
    // Constructor
    Tape();
    
    // Destructor
    ~Tape();
    
    // Reset tape state
    void reset();
    long long ticks;
    bool isTapePlayed;
    bool getNextBit();
};

#endif // TAPE_HPP
