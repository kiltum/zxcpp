#ifndef TAPE_HPP
#define TAPE_HPP

#include <cstdint>
#include <string>
#include <vector>

class Tape {
private:
    std::vector<uint8_t> tapeData;
    
public:
    // Constructor
    Tape();
    
    // Destructor
    ~Tape();
    
    // Reset tape state
    void reset();
    
    // Load tape file
    bool loadFile(const std::string& fileName);
    
    // Parse TAP file format
    void parseTap(const std::vector<uint8_t>& data);
    
    // Parse TZX file format
    void parseTzx(const std::vector<uint8_t>& data);
    
    long long ticks;
    bool isTapePlayed;
    bool getNextBit();
};

#endif // TAPE_HPP
