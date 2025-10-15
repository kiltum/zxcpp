#ifndef Z80_HPP
#define Z80_HPP

#include <cstdint>
#include "memory.hpp"

class Z80 {
private:
    Memory* memory;  // Pointer to memory instance

public:
    // Main register set
    union {
        struct {
            union {
                struct {
                    uint8_t F;
                    uint8_t A;
                };
                uint16_t AF;
            };
        };
        
        struct {
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
        };
    } registers;
    
    // Shadow register set (alternates)
    union {
        struct {
            union {
                struct {
                    uint8_t F_;
                    uint8_t A_;
                };
                uint16_t AF_;
            };
        };
        
        struct {
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
        };
    } registers_;
    
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
    
    // Public registers for easy access to main registers
    uint16_t& AF = registers.AF;
    uint8_t& A = registers.A;
    uint8_t& F = registers.F;
    
    uint16_t& BC = registers.BC;
    uint8_t& B = registers.B;
    uint8_t& C = registers.C;
    
    uint16_t& DE = registers.DE;
    uint8_t& D = registers.D;
    uint8_t& E = registers.E;
    
    uint16_t& HL = registers.HL;
    uint8_t& H = registers.H;
    uint8_t& L = registers.L;
    
    // Public registers for easy access to shadow registers
    uint16_t& AF_ = registers_.AF_;
    uint8_t& A_ = registers_.A_;
    uint8_t& F_ = registers_.F_;
    
    uint16_t& BC_ = registers_.BC_;
    uint8_t& B_ = registers_.B_;
    uint8_t& C_ = registers_.C_;
    
    uint16_t& DE_ = registers_.DE_;
    uint8_t& D_ = registers_.D_;
    uint8_t& E_ = registers_.E_;
    
    uint16_t& HL_ = registers_.HL_;
    uint8_t& H_ = registers_.H_;
    uint8_t& L_ = registers_.L_;
    
    // Constructor
    Z80(Memory* mem);
    
    // Execute one instruction and return number of ticks consumed
    int ExecuteOneInstruction();
};

#endif // Z80_HPP
