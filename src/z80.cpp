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
    SP = 0;
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
    memory->WriteWord(SP, value);
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
    uint8_t temp = A;
    uint8_t correction = 0;
    bool carry = GetFlag(FLAG_C);

    if (GetFlag(FLAG_H) || (A & 0x0F) > 9) {
        correction |= 0x06;
    }
    if (carry || A > 0x99) {
        correction |= 0x60;
    }

    if (GetFlag(FLAG_N)) {
        A -= correction;
    } else {
        A += correction;
    }

    SetFlag(FLAG_S, (A & 0x80) != 0);
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_H, ((temp ^ correction ^ A) & 0x10) != 0);
    SetFlag(FLAG_PV, parity(A));
    SetFlag(FLAG_C, carry || (correction & 0x60) != 0);
    // Set X and Y flags from result
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
    // https://worldofspectrum.org/forums/discussion/41704
    SetFlag(FLAG_C, true);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    // This sets both flags if BOTH bits 3 and 5 are set in A, otherwise clears both
    if ((A & FLAG_Y) == FLAG_Y && (A & FLAG_X) == FLAG_X) {
        SetFlag(FLAG_Y, true);
        SetFlag(FLAG_X, true);
    }
    // Note: If the condition is not met, the flags remain cleared (default behavior)
    // SetFlag(FLAG_C, true);
    // SetFlag(FLAG_N, false);
    // SetFlag(FLAG_H, false);
    // // FIX: X and Y flags come from A register
    // SetFlag(FLAG_X, (A & FLAG_X) != 0);
    // SetFlag(FLAG_Y, (A & FLAG_Y) != 0);
}

// ccf complements the carry flag
void Z80::ccf() {
    bool oldCarry = GetFlag(FLAG_C);
    SetFlag(FLAG_C, !oldCarry);
    SetFlag(FLAG_H, oldCarry); // H = old C
    SetFlag(FLAG_N, false);
    // test fuse pass, test zexall failed ?
    if ((A & FLAG_Y) == FLAG_Y && (A & FLAG_X) == FLAG_X) {
        SetFlag(FLAG_Y, true);
        SetFlag(FLAG_X, true);
    }
    // FIX: X and Y flags come from A register
    // SetFlag(FLAG_X, (A & FLAG_X) != 0);
    // SetFlag(FLAG_Y, (A & FLAG_Y) != 0);
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

// rlc rotates a byte left circular
uint8_t Z80::rlc(uint8_t value) {
    uint8_t result = (value << 1) | (value >> 7);
    SetFlag(FLAG_C, (value & 0x80) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// rrc rotates a byte right circular
uint8_t Z80::rrc(uint8_t value) {
    uint8_t result = (value >> 1) | (value << 7);
    SetFlag(FLAG_C, (value & 0x01) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// rl rotates a byte left through carry
uint8_t Z80::rl(uint8_t value) {
    bool oldCarry = GetFlag(FLAG_C);
    uint8_t result = (value << 1);
    if (oldCarry) {
        result |= 0x01;
    }
    SetFlag(FLAG_C, (value & 0x80) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// rr rotates a byte right through carry
uint8_t Z80::rr(uint8_t value) {
    bool oldCarry = GetFlag(FLAG_C);
    uint8_t result = (value >> 1);
    if (oldCarry) {
        result |= 0x80;
    }
    SetFlag(FLAG_C, (value & 0x01) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// sla shifts a byte left arithmetic
uint8_t Z80::sla(uint8_t value) {
    uint8_t result = value << 1;
    SetFlag(FLAG_C, (value & 0x80) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// sra shifts a byte right arithmetic
uint8_t Z80::sra(uint8_t value) {
    uint8_t result = (value >> 1) | (value & 0x80);
    SetFlag(FLAG_C, (value & 0x01) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// sll shifts a byte left logical (Undocumented)
uint8_t Z80::sll(uint8_t value) {
    uint8_t result = (value << 1) | 0x01;
    SetFlag(FLAG_C, (value & 0x80) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// srl shifts a byte right logical
uint8_t Z80::srl(uint8_t value) {
    uint8_t result = value >> 1;
    SetFlag(FLAG_C, (value & 0x01) != 0);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    UpdateSZXYPVFlags(result);
    return result;
}

// bit tests a bit in a byte
void Z80::bit(uint8_t bitNum, uint8_t value) {
    uint8_t mask = uint8_t(1 << bitNum);
    uint8_t result = value & mask;
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_Y, (value & (1 << 5)) != 0);
    SetFlag(FLAG_X, (value & (1 << 3)) != 0);
    SetFlag(FLAG_H, true);
    ClearFlag(FLAG_N);
    if (result == 0) {
        SetFlag(FLAG_PV, true);
        ClearFlag(FLAG_S);
    } else {
        ClearFlag(FLAG_PV);
        // For BIT 7, S flag is set to the value of bit 7
        if (bitNum == 7) {
            SetFlag(FLAG_S, (value & 0x80) != 0);
        } else {
            ClearFlag(FLAG_S);
        }
    }
}

// res resets a bit in a byte
uint8_t Z80::res(uint8_t bitNum, uint8_t value) {
    uint8_t mask = uint8_t(~(1 << bitNum));
    return value & mask;
}

// bitMem tests a bit in a byte for memory references
void Z80::bitMem(uint8_t bitNum, uint8_t value, uint8_t addrHi) {
    uint8_t mask = uint8_t(1 << bitNum);
    uint8_t result = value & mask;
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_Y, (addrHi & (1 << 5)) != 0);
    SetFlag(FLAG_X, (addrHi & (1 << 3)) != 0);
    SetFlag(FLAG_H, true);
    ClearFlag(FLAG_N);
    if (result == 0) {
        SetFlag(FLAG_PV, true);
        ClearFlag(FLAG_S);
    } else {
        ClearFlag(FLAG_PV);
        // For BIT 7, S flag is set to the value of bit 7
        if (bitNum == 7) {
            SetFlag(FLAG_S, (value & 0x80) != 0);
        } else {
            ClearFlag(FLAG_S);
        }
    }
}

// set sets a bit in a byte
uint8_t Z80::set(uint8_t bitNum, uint8_t value) {
    uint8_t mask = uint8_t(1 << bitNum);
    return value | mask;
}

// add16IX adds two 16-bit values for IX register and updates flags
uint16_t Z80::add16IX(uint16_t a, uint16_t b) {
    uint32_t result = (uint32_t)a + (uint32_t)b;
    SetFlag(FLAG_C, result > 0xFFFF);
    SetFlag(FLAG_H, (a & 0x0FFF) + (b & 0x0FFF) > 0x0FFF);
    ClearFlag(FLAG_N);
    // For IX operations, we update X and Y flags from high byte of result
    UpdateFlags3and5FromAddress((uint16_t)result);
    return (uint16_t)result;
}

// Get IXH (high byte of IX)
uint8_t Z80::GetIXH() {
    return uint8_t(IX >> 8);
}

// Get IXL (low byte of IX)
uint8_t Z80::GetIXL() {
    return uint8_t(IX & 0xFF);
}

// Set IXH (high byte of IX)
void Z80::SetIXH(uint8_t value) {
    IX = (IX & 0x00FF) | (uint16_t(value) << 8);
}

// Set IXL (low byte of IX)
void Z80::SetIXL(uint8_t value) {
    IX = (IX & 0xFF00) | uint16_t(value);
}

// executeIncDecIndexed handles INC/DEC (IX+d) instructions
int Z80::executeIncDecIndexed(bool isInc) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IX) + int32_t(displacement));
    uint8_t value = memory->ReadByte(addr);
    uint8_t result;
    if (isInc) {
        result = inc8(value);
    } else {
        result = dec8(value);
    }
    memory->WriteByte(addr, result);
    MEMPTR = addr;
    return 23;
}

// executeLoadFromIndexed handles LD r, (IX+d) instructions
int Z80::executeLoadFromIndexed(uint8_t reg) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IX) + int32_t(displacement));
    uint8_t value = memory->ReadByte(addr);

    switch (reg) {
    case 0:
        B = value;
        break;
    case 1:
        C = value;
        break;
    case 2:
        D = value;
        break;
    case 3:
        E = value;
        break;
    case 4:
        H = value;
        break;
    case 5:
        L = value;
        break;
    case 7:
        A = value;
        break;
    }

    MEMPTR = addr;
    return 19;
}

// executeStoreToIndexed handles LD (IX+d), r instructions
int Z80::executeStoreToIndexed(uint8_t value) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IX) + int32_t(displacement));
    memory->WriteByte(addr, value);
    MEMPTR = addr;
    return 19;
}

// executeALUIndexed handles ALU operations with (IX+d) operand
int Z80::executeALUIndexed(uint8_t opType) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IX) + int32_t(displacement));
    uint8_t value = memory->ReadByte(addr);

    switch (opType) {
    case 0: // ADD
        add8(value);
        break;
    case 1: // ADC
        adc8(value);
        break;
    case 2: // SUB
        sub8(value);
        break;
    case 3: // SBC
        sbc8(value);
        break;
    case 4: // AND
        and8(value);
        break;
    case 5: // XOR
        xor8(value);
        break;
    case 6: // OR
        or8(value);
        break;
    case 7: // CP
        cp8(value);
        break;
    }

    MEMPTR = addr;
    return 19;
}

// executeDDCBOpcode handles DD CB prefix (IX with displacement and CB operations)
int Z80::executeDDCBOpcode() {
    // This function will be implemented in z80_ddcb_opcodes.cpp
    // For now, we'll just return a default value
    return 0;
}
