#include "z80.hpp"
#include "memory.hpp"
#include "port.hpp"

Z80::Z80(Memory* mem, Port* port) {
    // Store the memory and port pointers
    memory = mem;
    this->port = port;
    
    // Initialize main registers to zero
    AF = 0;
    BC = 0;
    DE = 0;
    HL = 0;
    
    // Initialize shadow registers to zero
    AF_ = 0;
    BC_ = 0;
    DE_ = 0;
    HL_ = 0;
    
    // Initialize index registers to zero
    IX = 0;
    IY = 0;
    
    // Initialize internal registers to zero
    SP = 0xFFFF;
    PC = 0;
    I = 0;
    R = 0;
    MEMPTR = 0;
    
    // Initialize interrupt-related registers and flags
    IFF1 = false;
    IFF2 = false;
    IM = 0;
    HALT = false;
    InterruptPending = false;
}

int Z80::ExecuteOneInstruction() {
    // Handle interrupts first if enabled
    if (IFF1 && InterruptPending) {
        InterruptPending = false;
        return HandleInterrupt();
    }
    InterruptPending = false;
    
    // Handle HALT state
    if (HALT) {
        return 4; // 4 T-states for HALT
    }

    // Read the first opcode byte
    uint8_t opcode = memory->ReadByte(PC);
    
    // Handle prefix opcodes
    switch (opcode) {
        case 0xDD: // DD prefix (IX instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteDDOpcode();
            
        case 0xFD: // FD prefix (IY instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteFDOpcode();
            
        case 0xCB: // CB prefix (bit manipulation instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteCBOpcode();
            
        case 0xED: // ED prefix (extended instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteEDOpcode();
            
        default: // Regular opcode
            return ExecuteOpcode();
    }
}

// HandleInterrupt handles interrupt processing
int Z80::HandleInterrupt() {
    // Exit HALT state
    // The HALT instruction halts the Z80; it does not increase the PC so that the instruction is re-
    // executed, until a maskable or non-maskable interrupt is accepted. Only then does the Z80 increase
    // the PC again and continues with the next instruction. During the HALT state, the HALT line is
    // set. The PC is increased before the interrupt routine is called.
    if (HALT) {
        HALT = false;
        PC++;  // Increment PC to exit HALT state
    }
    
    // Reset interrupt flip-flops
    IFF1 = false;
    IFF2 = false;
    
    // Handle interrupt based on mode
    switch (IM) {
        case 0:
        case 1:
            // Mode 0/1: Restart at address 0x0038
            Push(PC);
            PC = 0x0038;
            return 13; // 13 T-states for interrupt handling
            
        case 2:
            // Mode 2: Call interrupt vector
            Push(PC);
            {
                uint16_t vectorAddr = (uint16_t(I) << 8) | 0xFF; // Use 0xFF as vector for non-maskable interrupt
                PC = (uint16_t(memory->ReadByte(vectorAddr + 1)) << 8) | uint16_t(memory->ReadByte(vectorAddr));
            }
            return 19; // 19 T-states for interrupt handling
            
        default:
            // Should not happen, but handle gracefully
            return 0;
    }
}

// UpdateSZFlags updates the S and Z flags based on an 8-bit result
void Z80::UpdateSZFlags(uint8_t result) {
    SetFlag(FLAG_S, (result & 0x80) != 0);
    SetFlag(FLAG_Z, result == 0);
}

// UpdatePVFlags updates the P/V flag based on an 8-bit result (parity calculation)
void Z80::UpdatePVFlags(uint8_t result) {
    // Calculate parity (even number of 1-bits = 1, odd = 0)
    uint8_t parity = 1;
    for (int i = 0; i < 8; i++) {
        parity ^= (result >> i) & 1;
    }
    SetFlag(FLAG_PV, parity != 0);
}

// UpdateSZXYPVFlags updates the S, Z, X, Y, P/V flags based on an 8-bit result
void Z80::UpdateSZXYPVFlags(uint8_t result) {
    SetFlag(FLAG_S, (result & 0x80) != 0);
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_X, (result & FLAG_X) != 0);
    SetFlag(FLAG_Y, (result & FLAG_Y) != 0);

    // Calculate parity (even number of 1-bits = 1, odd = 0)
    uint8_t parity = 1;
    for (int i = 0; i < 8; i++) {
        parity ^= (result >> i) & 1;
    }
    SetFlag(FLAG_PV, parity != 0);
}

// UpdateFlags3and5FromValue updates the X and Y flags from an 8-bit value
void Z80::UpdateFlags3and5FromValue(uint8_t value) {
    SetFlag(FLAG_X, (value & FLAG_X) != 0);
    SetFlag(FLAG_Y, (value & FLAG_Y) != 0);
}

// UpdateFlags3and5FromAddress updates the X and Y flags from the high byte of an address
void Z80::UpdateFlags3and5FromAddress(uint16_t address) {
    SetFlag(FLAG_X, ((address >> 8) & FLAG_X) != 0);
    SetFlag(FLAG_Y, ((address >> 8) & FLAG_Y) != 0);
}

// UpdateSZXYFlags updates the S, Z, X, Y flags based on an 8-bit result
void Z80::UpdateSZXYFlags(uint8_t result) {
    SetFlag(FLAG_S, (result & 0x80) != 0);
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_X, (result & FLAG_X) != 0);
    SetFlag(FLAG_Y, (result & FLAG_Y) != 0);
}

// UpdateXYFlags updates the undocumented X and Y flags based on an 8-bit result
void Z80::UpdateXYFlags(uint8_t result) {
    SetFlag(FLAG_X, (result & FLAG_X) != 0);
    SetFlag(FLAG_Y, (result & FLAG_Y) != 0);
}

// GetFlag returns the state of a specific flag
bool Z80::GetFlag(uint8_t flag) {
    return (F & flag) != 0;
}

// SetFlag sets a flag to a specific state
void Z80::SetFlag(uint8_t flag, bool state) {
    if (state) {
        F |= flag;
    } else {
        F &= ~flag;
    }
}

// ClearFlag clears a specific flag
void Z80::ClearFlag(uint8_t flag) {
    F &= ~flag;
}

// ClearAllFlags clears all flags
void Z80::ClearAllFlags() {
    F = 0;
}


// ReadImmediateByte reads the next byte from memory at PC and increments PC
uint8_t Z80::ReadImmediateByte() {
    uint8_t value = memory->ReadByte(PC);
    PC++;
    return value;
}

// ReadImmediateWord reads the next word from memory at PC and increments PC by 2
uint16_t Z80::ReadImmediateWord() {
    uint8_t lo = memory->ReadByte(PC);
    PC++;
    uint8_t hi = memory->ReadByte(PC);
    PC++;
    return (uint16_t(hi) << 8) | uint16_t(lo);
}

// ReadDisplacement reads an 8-bit signed displacement value
int8_t Z80::ReadDisplacement() {
    int8_t value = int8_t(memory->ReadByte(PC));
    PC++;
    return value;
}

// ReadOpcode reads the next opcode from memory at PC and increments PC
uint8_t Z80::ReadOpcode() {
    uint8_t opcode = memory->ReadByte(PC);
    PC++;
    // Increment R register (memory refresh) for each opcode fetch
    // Note: R is a 7-bit register, bit 7 remains unchanged
    R = (R & 0x80) | ((R + 1) & 0x7F);
    return opcode;
}

// Push pushes a 16-bit value onto the stack
void Z80::Push(uint16_t value) {
    SP -= 2;
    memory->WriteByte(SP, uint8_t(value & 0xFF));
    memory->WriteByte(SP + 1, uint8_t((value >> 8) & 0xFF));
}

// Pop pops a 16-bit value from the stack
uint16_t Z80::Pop() {
    // Read low byte first, then high byte (little-endian)
    uint8_t lo = memory->ReadByte(SP);
    uint8_t hi = memory->ReadByte(SP + 1);
    SP += 2;
    return (uint16_t(hi) << 8) | uint16_t(lo);
}

// inc8 increments an 8-bit value and updates flags
uint8_t Z80::inc8(uint8_t value) {
    uint8_t result = value + 1;
    SetFlag(FLAG_H, (value & 0x0F) == 0x0F);
    SetFlag(FLAG_N, false);
    UpdateSZXYFlags(result);
    // Set PV flag if incrementing 0x7F to 0x80 (overflow from positive to negative)
    SetFlag(FLAG_PV, value == 0x7F);
    return result;
}

// dec8 decrements an 8-bit value and updates flags
uint8_t Z80::dec8(uint8_t value) {
    uint8_t result = value - 1;
    SetFlag(FLAG_H, (value & 0x0F) == 0x00);
    SetFlag(FLAG_N, true);
    UpdateSZXYFlags(result);
    // Set PV flag if decrementing 0x80 to 0x7F (overflow from negative to positive)
    SetFlag(FLAG_PV, value == 0x80);
    return result;
}

// rlca rotates the accumulator left circular
void Z80::rlca() {
    uint8_t result = (A << 1) | (A >> 7);
    A = result;
    SetFlag(FLAG_C, (A & 0x01) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    UpdateXYFlags(A);
}

// rla rotates the accumulator left through carry
void Z80::rla() {
    bool oldCarry = GetFlag(FLAG_C);
    uint8_t result = (A << 1);
    if (oldCarry) {
        result |= 0x01;
    }
    SetFlag(FLAG_C, (A & 0x80) != 0);
    A = result;
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    UpdateXYFlags(A);
}

// rrca rotates the accumulator right circular
void Z80::rrca() {
    uint8_t result = (A >> 1) | (A << 7);
    A = result;
    SetFlag(FLAG_C, (A & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    UpdateXYFlags(A);
}

// rra rotates the accumulator right through carry
void Z80::rra() {
    bool oldCarry = GetFlag(FLAG_C);
    uint8_t result = (A >> 1);
    if (oldCarry) {
        result |= 0x80;
    }
    SetFlag(FLAG_C, (A & 0x01) != 0);
    A = result;
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    UpdateXYFlags(A);
}

// daa performs decimal adjust on accumulator
void Z80::daa() {
    uint8_t correction = 0;

    if (GetFlag(FLAG_H) || (A & 0x0F) > 9) {
        correction += 0x06;
    }

    if (A > 0x99 || GetFlag(FLAG_C)) {
        correction += 0x60;
        SetFlag(FLAG_C, true);
    }

    bool isSubtraction = GetFlag(FLAG_N);
    if (isSubtraction) {
        SetFlag(FLAG_H, GetFlag(FLAG_H) && (A & 0x0F) < 0x06);
        A -= correction;
    } else {
        SetFlag(FLAG_H, (A & 0x0F) > 9);
        A += correction;
    }

    SetFlag(FLAG_S, (A & 0x80) != 0);
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_PV, parity(A));
    SetFlag(FLAG_X, (A & FLAG_X) != 0);
    SetFlag(FLAG_Y, (A & FLAG_Y) != 0);
}

// Helper function to calculate parity
bool Z80::parity(uint8_t val) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (val & (1 << i)) {
            count++;
        }
    }
    return (count % 2) == 0;
}

// cpl complements the accumulator
void Z80::cpl() {
    A = ~A;
    SetFlag(FLAG_H, true);
    SetFlag(FLAG_N, true);
    UpdateXYFlags(A);
}

// scf sets the carry flag
void Z80::scf() {
    SetFlag(FLAG_C, true);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_X, (A & FLAG_X) != 0);
    SetFlag(FLAG_Y, (A & FLAG_Y) != 0);
}

// ccf complements the carry flag
void Z80::ccf() {
    bool oldCarry = GetFlag(FLAG_C);
    SetFlag(FLAG_C, !oldCarry);
    SetFlag(FLAG_H, oldCarry); // H = old C
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_X, (A & FLAG_X) != 0);
    SetFlag(FLAG_Y, (A & FLAG_Y) != 0);
}

// add16 adds two 16-bit values and updates flags
uint16_t Z80::add16(uint16_t a, uint16_t b) {
    uint32_t result = (uint32_t)a + (uint32_t)b;
    SetFlag(FLAG_C, result > 0xFFFF);
    SetFlag(FLAG_H, (a & 0x0FFF) + (b & 0x0FFF) > 0x0FFF);
    SetFlag(FLAG_N, false);
    UpdateFlags3and5FromAddress((uint16_t)result);
    return (uint16_t)result;
}

// add8 adds an 8-bit value to the accumulator and updates flags
void Z80::add8(uint8_t value) {
    uint8_t a = A;
    uint16_t result = (uint16_t)a + (uint16_t)value;
    SetFlag(FLAG_C, result > 0xFF);
    SetFlag(FLAG_H, (a & 0x0F) + (value & 0x0F) > 0x0F);
    SetFlag(FLAG_N, false);
    UpdateSZXYFlags((uint8_t)result);
    // Set overflow flag: overflow occurs if adding two numbers with the same sign
    // produces a result with a different sign
    // Both operands have the same sign (both positive or both negative)
    // but the result has a different sign
    bool sameSign = ((a ^ value) & 0x80) == 0;
    bool differentResultSign = ((a ^ result) & 0x80) != 0;
    bool overflow = sameSign && differentResultSign;
    SetFlag(FLAG_PV, overflow);
    A = (uint8_t)result;
}

// adc8 adds an 8-bit value and carry to the accumulator and updates flags
void Z80::adc8(uint8_t value) {
    uint8_t a = A;
    bool carry = GetFlag(FLAG_C);
    uint16_t result;
    if (carry) {
        result = (uint16_t)a + (uint16_t)value + 1;
    } else {
        result = (uint16_t)a + (uint16_t)value;
    }
    SetFlag(FLAG_C, (int)a + (int)value + (int)(carry ? 1 : 0) > 0xFF);
    SetFlag(FLAG_H, (a & 0x0F) + (value & 0x0F) + (carry ? 1 : 0) > 0x0F);
    SetFlag(FLAG_N, false);
    UpdateSZXYFlags((uint8_t)result);
    // Set overflow flag: overflow occurs if adding two numbers with the same sign
    // produces a result with a different sign
    uint8_t originalValue = a;
    if (carry) {
        bool sameSign = ((originalValue ^ value) & 0x80) == 0;
        bool differentResultSign = ((originalValue ^ result) & 0x80) != 0;
        bool overflow = sameSign && differentResultSign;
        SetFlag(FLAG_PV, overflow);
    } else {
        bool sameSign = ((originalValue ^ value) & 0x80) == 0;
        bool differentResultSign = ((originalValue ^ result) & 0x80) != 0;
        bool overflow = sameSign && differentResultSign;
        SetFlag(FLAG_PV, overflow);
    }
    A = (uint8_t)result;
}

// sub8 subtracts an 8-bit value from the accumulator and updates flags
void Z80::sub8(uint8_t value) {
    uint8_t a = A;
    uint16_t result = (uint16_t)a - (uint16_t)value;
    SetFlag(FLAG_C, a < value);
    SetFlag(FLAG_H, (a & 0x0F) < (value & 0x0F));
    SetFlag(FLAG_N, true);
    UpdateSZXYFlags((uint8_t)result);
    // Set overflow flag: overflow occurs if subtracting two numbers with different signs
    // produces a result with the same sign as the subtrahend
    bool overflow = (((a ^ value) & 0x80) != 0) && (((a ^ result) & 0x80) != 0);
    SetFlag(FLAG_PV, overflow);
    A = (uint8_t)result;
}

// sbc8 subtracts an 8-bit value and carry from the accumulator and updates flags
void Z80::sbc8(uint8_t value) {
    uint8_t a = A;
    bool carry = GetFlag(FLAG_C);
    uint16_t result;
    if (carry) {
        result = (uint16_t)a - (uint16_t)value - 1;
    } else {
        result = (uint16_t)a - (uint16_t)value;
    }
    SetFlag(FLAG_C, (int)a - (int)value - (int)(carry ? 1 : 0) < 0);
    SetFlag(FLAG_H, (a & 0x0F) < (value & 0x0F) + (carry ? 1 : 0));
    SetFlag(FLAG_N, true);
    UpdateSZXYFlags((uint8_t)result);
    // Set overflow flag: overflow occurs if subtracting two numbers with different signs
    // produces a result with the same sign as the subtrahend
    uint8_t originalValue = a;
    if (carry) {
        bool overflow = (((originalValue ^ value) & 0x80) != 0) && (((originalValue ^ result) & 0x80) != 0);
        SetFlag(FLAG_PV, overflow);
    } else {
        bool overflow = (((originalValue ^ value) & 0x80) != 0) && (((originalValue ^ result) & 0x80) != 0);
        SetFlag(FLAG_PV, overflow);
    }
    A = (uint8_t)result;
}

// and8 performs bitwise AND with the accumulator and updates flags
void Z80::and8(uint8_t value) {
    A &= value;
    SetFlag(FLAG_C, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, true);
    UpdateSZXYFlags(A);
    // For logical operations, P/V flag indicates parity
    SetFlag(FLAG_PV, parity(A));
}

// xor8 performs bitwise XOR with the accumulator and updates flags
void Z80::xor8(uint8_t value) {
    A ^= value;
    SetFlag(FLAG_C, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    UpdateSZXYFlags(A);
    // For logical operations, P/V flag indicates parity
    SetFlag(FLAG_PV, parity(A));
}

// or8 performs bitwise OR with the accumulator and updates flags
void Z80::or8(uint8_t value) {
    A |= value;
    SetFlag(FLAG_C, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    UpdateSZXYFlags(A);
    // For logical operations, P/V flag indicates parity
    SetFlag(FLAG_PV, parity(A));
}

// cp8 compares an 8-bit value with the accumulator and updates flags
void Z80::cp8(uint8_t value) {
    uint8_t a = A;
    uint16_t result = (uint16_t)a - (uint16_t)value;
    SetFlag(FLAG_C, a < value);
    SetFlag(FLAG_H, (a & 0x0F) < (value & 0x0F));
    SetFlag(FLAG_N, true);
    UpdateSZFlags((uint8_t)result);
    // For CP instruction, X and Y flags are set from the operand, not the result
    UpdateFlags3and5FromValue(value);
    // Set overflow flag: overflow occurs if subtracting two numbers with different signs
    // produces a result with the same sign as the subtrahend
    bool overflow = (((a ^ value) & 0x80) != 0) && (((a ^ result) & 0x80) != 0);
    SetFlag(FLAG_PV, overflow);
}
