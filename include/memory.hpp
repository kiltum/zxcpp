#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

class Memory
{
private:
    uint8_t bank[8][16384]; // banks of memory
    uint8_t rom[3][16384];  // banks of ROMs. 0 - zxspectrum 48 or 1st rom of 128. 1 - second rom of 128. 2 - trdos rom
    bool is48;              // machine version
    uint8_t bankMapping[4]; // Which bank mapped now
    bool ULAShadow;         // is ULA read from shadow rom?
    uint8_t isTrDos;           // is TR DOS rom enabled?

public:
    // Constructor
    Memory();

    // Read a byte from memory
    uint8_t ReadByte(uint16_t address);
    // Special function for ULA reading screen (main or shadow)
    uint8_t ULAReadByte(uint16_t address);
    // Write a byte to memory
    void WriteByte(uint16_t address, uint8_t value);

    // Load 48k rom to memory
    void Read48(void);
    // load 128 rom to memory
    void Read128(void);
    // Load diag rom to memory
    void ReadDiag(void);
    // load another diag rom
    void ReadDiag2(void);

    // true, is we emulate 48k. No banks, no any reaction to write to 7FFD. false - we emulate 128k
    void change48(bool is48s);
    void writePort(uint16_t port, uint8_t value); // handler for 7ffd
    bool getIs48() const { return is48; }         // Getter for is48 flag
    bool canWriteRom;                             // Can we overwrite ROM, as in Baltika version? Its public, because test need it
    void enableTrDos(bool is);                    // enable trdos rom or not
    bool checkTrDos(void);
    void Clear(void);
};

#endif // MEMORY_HPP
