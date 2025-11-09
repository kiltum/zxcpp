#ifndef Z80_HPP
#define Z80_HPP

#include <cstdint>
#include "memory.hpp"
#include "port.hpp"

// Z80 Flag Definitions
#define FLAG_S 0x80  // Sign Flag (S) - bit 7
#define FLAG_Z 0x40  // Zero Flag (Z) - bit 6
#define FLAG_Y 0x20  // Bit 5 (Y) - bit 5
#define FLAG_H 0x10  // Half Carry Flag (H) - bit 4
#define FLAG_X 0x08  // Bit 3 (X) - bit 3
#define FLAG_PV 0x04 // Parity/Overflow Flag (P/V) - bit 2
#define FLAG_N 0x02  // Add/Subtract Flag (N) - bit 1
#define FLAG_C 0x01  // Carry Flag (C) - bit 0

class Z80
{
public:
    Memory *memory; // Pointer to memory instance
    Port *port;     // Pointer to port instance

    // Main register set
    union
    {
        struct
        {
            uint8_t F;
            uint8_t A;
        };
        uint16_t AF;
    };

    union
    {
        struct
        {
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;
    };

    union
    {
        struct
        {
            uint8_t E;
            uint8_t D;
        };
        uint16_t DE;
    };

    union
    {
        struct
        {
            uint8_t L;
            uint8_t H;
        };
        uint16_t HL;
    };

    // Shadow register set (alternates)
    union
    {
        struct
        {
            uint8_t F_;
            uint8_t A_;
        };
        uint16_t AF_;
    };

    union
    {
        struct
        {
            uint8_t C_;
            uint8_t B_;
        };
        uint16_t BC_;
    };

    union
    {
        struct
        {
            uint8_t E_;
            uint8_t D_;
        };
        uint16_t DE_;
    };

    union
    {
        struct
        {
            uint8_t L_;
            uint8_t H_;
        };
        uint16_t HL_;
    };

    // Index registers
    uint16_t IX;
    uint16_t IY;

    // Internal registers
    uint16_t SP;     // Stack Pointer
    uint16_t PC;     // Program Counter
    uint8_t I;       // Interrupt vector
    uint8_t R;       // Refresh counter
    uint16_t MEMPTR; // Memory pointer (internal use)

    // Interrupt-related registers and flags
    bool IFF1;             // Interrupt Flip Flop 1
    bool IFF2;             // Interrupt Flip Flop 2
    uint8_t IM;            // Interrupt Mode (0, 1, or 2)
    bool HALT;             // HALT state flag
    bool InterruptPending; // Interrupt pending flag

    // Constructor
    Z80(Memory *mem, Port *port);

    void Reset(void);

    // Execute one instruction and return number of ticks consumed
    int ExecuteOneInstruction();

    // Handle interrupt processing
    int HandleInterrupt();
    bool isNMOS;    // cpu type NMOS (true, default) or Zilog/SGS (false)
    void NMI(void); // Non Maskable Interrupt

private:
    // Flag update functions
    void UpdateSZFlags(uint8_t result);
    void UpdatePVFlags(uint8_t result);
    void UpdateSZXYPVFlags(uint8_t result);
    void UpdateFlags3and5FromValue(uint8_t value);
    void UpdateFlags3and5FromAddress(uint16_t address);
    void UpdateSZXYFlags(uint8_t result);
    void UpdateXYFlags(uint8_t result);

    // Flag manipulation functions
    bool GetFlag(uint8_t flag);
    void SetFlag(uint8_t flag, bool state);
    void ClearFlag(uint8_t flag);
    void ClearAllFlags();

    // Helper functions
    uint8_t ReadImmediateByte();
    uint16_t ReadImmediateWord();
    int8_t ReadDisplacement();
    uint8_t ReadOpcode();
    void Push(uint16_t value);
    uint16_t Pop();

    // Opcode helper functions
    uint8_t inc8(uint8_t value);
    uint8_t dec8(uint8_t value);
    void rlca();
    void rla();
    void rrca();
    void rra();
    void daa();
    bool parity(uint8_t val);
    void cpl();
    void scf();
    void ccf();
    uint16_t add16(uint16_t a, uint16_t b);
    void add8(uint8_t value);
    void adc8(uint8_t value);
    void sub8(uint8_t value);
    void sbc8(uint8_t value);
    void and8(uint8_t value);
    void xor8(uint8_t value);
    void or8(uint8_t value);
    void cp8(uint8_t value);

    // CB opcode helper functions
    uint8_t rlc(uint8_t value);
    uint8_t rrc(uint8_t value);
    uint8_t rl(uint8_t value);
    uint8_t rr(uint8_t value);
    uint8_t sla(uint8_t value);
    uint8_t sra(uint8_t value);
    uint8_t sll(uint8_t value);
    uint8_t srl(uint8_t value);
    void bit(uint8_t bitNum, uint8_t value);
    void bitMem(uint8_t bitNum, uint8_t value, uint8_t addrHi);
    uint8_t res(uint8_t bitNum, uint8_t value);
    uint8_t set(uint8_t bitNum, uint8_t value);

    // DD opcode helper functions
    uint16_t add16IX(uint16_t a, uint16_t b);
    uint8_t GetIXH();
    uint8_t GetIXL();
    void SetIXH(uint8_t value);
    void SetIXL(uint8_t value);
    int executeIncDecIndexed(bool isInc);
    int executeLoadFromIndexed(uint8_t reg);
    int executeStoreToIndexed(uint8_t value);
    int executeALUIndexed(uint8_t opType);
    int executeDDCBOpcode();

    // FD opcode helper functions
    uint16_t add16IY(uint16_t a, uint16_t b);
    uint8_t GetIYH();
    uint8_t GetIYL();
    void SetIYH(uint8_t value);
    void SetIYL(uint8_t value);
    int executeIncDecIndexedIY(bool isInc);
    int executeLoadFromIndexedIY(uint8_t reg);
    int executeStoreToIndexedIY(uint8_t value);
    int executeALUIndexedIY(uint8_t opType);

    // FDCB opcode helper functions
    int executeRotateShiftIndexedIY(uint8_t opcode, uint16_t addr, uint8_t value);
    int executeResetBitIndexedIY(uint8_t opcode, uint16_t addr, uint8_t value);
    int executeSetBitIndexedIY(uint8_t opcode, uint16_t addr, uint8_t value);

    // ED opcode helper functions
    uint16_t sbc16(uint16_t val1, uint16_t val2);
    uint16_t sbc16WithMEMPTR(uint16_t a, uint16_t b);
    uint16_t adc16(uint16_t val1, uint16_t val2);
    uint16_t adc16WithMEMPTR(uint16_t a, uint16_t b);
    void neg();
    void retn();
    void reti();
    void ldAI();
    void ldAR();
    void rrd();
    void rld();
    int executeIN(uint8_t reg);
    int executeOUT(uint8_t reg);
    void ldi();
    void cpi();
    void ini();
    void outi();
    void ldd();
    void cpd();
    void ind();
    void outd();
    int ldir();
    int cpir();
    int inir();
    int otir();
    int lddr();
    int cpdr();
    int indr();
    int otdr();
    uint8_t inC();
    void outC(uint8_t value);

    int ExecuteOpcode();
    int ExecuteCBOpcode();
    int ExecuteDDOpcode();
    int ExecuteEDOpcode();
    int ExecuteFDCBOpcode();
    int ExecuteFDOpcode();
};

#endif // Z80_HPP
