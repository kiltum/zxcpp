#ifndef Z80_HPP
#define Z80_HPP

#include <cstdint>
#include "memory.hpp"
#include "port.hpp"

// Z80 Flag Definitions
#define FLAG_S  0x80  // Sign Flag (S) - bit 7
#define FLAG_Z  0x40  // Zero Flag (Z) - bit 6
#define FLAG_Y  0x20  // Bit 5 (Y) - bit 5
#define FLAG_H  0x10  // Half Carry Flag (H) - bit 4
#define FLAG_X  0x08  // Bit 3 (X) - bit 3
#define FLAG_PV 0x04  // Parity/Overflow Flag (P/V) - bit 2
#define FLAG_N  0x02  // Add/Subtract Flag (N) - bit 1
#define FLAG_C  0x01  // Carry Flag (C) - bit 0

class Z80 {
public:
    Memory* memory;  // Pointer to memory instance
    Port* port;      // Pointer to port instance

    // Main register set
    union {
        struct {
            uint8_t F;
            uint8_t A;
        };
        uint16_t AF;
    };
    
    union {
        struct {
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;
    };
    
    union {
        struct {
            uint8_t E;
            uint8_t D;
        };
        uint16_t DE;
    };
    
    union {
        struct {
            uint8_t L;
            uint8_t H;
        };
        uint16_t HL;
    };
    
    // Shadow register set (alternates)
    union {
        struct {
            uint8_t F_;
            uint8_t A_;
        };
        uint16_t AF_;
    };
    
    union {
        struct {
            uint8_t C_;
            uint8_t B_;
        };
        uint16_t BC_;
    };
    
    union {
        struct {
            uint8_t E_;
            uint8_t D_;
        };
        uint16_t DE_;
    };
    
    union {
        struct {
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
    bool IFF1;       // Interrupt Flip Flop 1
    bool IFF2;       // Interrupt Flip Flop 2
    uint8_t IM;      // Interrupt Mode (0, 1, or 2)
    bool HALT;       // HALT state flag
    bool InterruptPending; // Interrupt pending flag
    
    // Constructor
    Z80(Memory* mem, Port* port);
    
    // Execute one instruction and return number of ticks consumed
    int ExecuteOneInstruction();
    
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
    
private:
};

#endif // Z80_HPP
