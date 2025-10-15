#ifndef Z80_HPP
#define Z80_HPP

#include <cstdint>
#include "memory.hpp"
#include "port.hpp"

class Z80 {
private:
    Memory* memory;  // Pointer to memory instance
    Port* port;      // Pointer to port instance

public:
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
};

#endif // Z80_HPP
