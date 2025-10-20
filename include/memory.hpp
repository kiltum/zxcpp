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
    
    // Read a word (16-bit) from memory (little-endian)
    uint16_t ReadWord(uint16_t address);
    
    // Write a word (16-bit) to memory (little-endian)
    void WriteWord(uint16_t address, uint16_t value);
    // Load 48k rom to memory
    void Read48(void);
    // Load diag rom to memory
    void ReadDiag(void);
    // load another diag rom
    void ReadDiag2(void);
};

#endif // MEMORY_HPP
