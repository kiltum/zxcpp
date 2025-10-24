#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

class Memory {
private:
    uint8_t memory[65536]; // 64KB of memory
    

public:
    // Constructor
    Memory();
    
    // Read a byte from memory
    uint8_t ReadByte(uint16_t address);
    
    // Write a byte to memory
    void WriteByte(uint16_t address, uint8_t value);
    
    // Load 48k rom to memory
    void Read48(void);
    // Load diag rom to memory
    void ReadDiag(void);
    // load another diag rom
    void ReadDiag2(void);
    bool canWriteRom; // Can we overwrite ROM, as in Baltika version?
};

#endif // MEMORY_HPP
