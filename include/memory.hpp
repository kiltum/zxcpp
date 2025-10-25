#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

class Memory {
private:
    uint8_t bank[7][16384]; // banks of memory
    uint8_t rom[2][16384]; // banks of ROMs
    bool is48; // machine version
    uint8_t bankMapping[4]; // Which bank mapped now 

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
    // true, is we emulate 48k. No banks, no any reaction to write to 7FFD. false - we emulate 128k
    void change48(bool is48s); 
    void writePort(uint16_t port, uint8_t value); // handler for 7ffd
};

#endif // MEMORY_HPP
