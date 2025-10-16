#include "z80.hpp"
#include "memory.hpp"
#include "port.hpp"
#include "z80_opcodes.hpp"
#include "z80_dd_opcodes.hpp"
#include "z80_fd_opcodes.hpp"
#include "z80_cb_opcodes.hpp"
#include "z80_ed_opcodes.hpp"
#include "z80_fdcb_opcodes.hpp"

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
            return ExecuteDDOpcode(this);
            
        case 0xFD: // FD prefix (IY instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteFDOpcode(this);
            
        case 0xCB: // CB prefix (bit manipulation instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteCBOpcode(this);
            
        case 0xED: // ED prefix (extended instructions)
            // Increment PC past the prefix
            PC++;
            return ExecuteEDOpcode(this);
            
        default: // Regular opcode
            return ExecuteOpcode(this);
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
