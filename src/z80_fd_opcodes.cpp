#include "z80.hpp"
#include "memory.hpp"

// Implementation of FD prefixed Z80 opcodes (IY instructions)
int Z80::ExecuteFDOpcode() {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = ReadOpcode();
    // R should not be incremented twice (already incremented in ExecuteOneInstruction for FD prefix)
    //R = (R & 0x80) | ((R - 1) & 0x7F);
    R++;
    switch (opcode) {
    // Load instructions
    case 0x09: // ADD IY, BC
        {
            uint16_t oldIY = IY;
            uint16_t result = add16IY(IY, BC);
            MEMPTR = oldIY + 1;
            IY = result;
        }
        return 15;
    case 0x19: // ADD IY, DE
        {
            uint16_t oldIY = IY;
            uint16_t result = add16IY(IY, DE);
            MEMPTR = oldIY + 1;
            IY = result;
        }
        return 15;
    case 0x21: // LD IY, nn
        IY = ReadImmediateWord();
        return 14;
    case 0x22: // LD (nn), IY
        {
            uint16_t addr = ReadImmediateWord();
            memory->WriteWord(addr, IY);
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x23: // INC IY
        IY++;
        return 10;
    case 0x24: // INC IYH
        SetIYH(inc8(GetIYH()));
        return 8;
    case 0x25: // DEC IYH
        SetIYH(dec8(GetIYH()));
        return 8;
    case 0x26: // LD IYH, n
        SetIYH(ReadImmediateByte());
        return 11;
    case 0x29: // ADD IY, IY
        {
            uint16_t oldIY = IY;
            uint16_t result = add16IY(IY, IY);
            MEMPTR = oldIY + 1;
            IY = result;
        }
        return 15;
    case 0x2A: // LD IY, (nn)
        {
            uint16_t addr = ReadImmediateWord();
            IY = memory->ReadWord(addr);
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x2B: // DEC IY
        IY--;
        return 10;
    case 0x2C: // INC IYL
        SetIYL(inc8(GetIYL()));
        return 8;
    case 0x2D: // DEC IYL
        SetIYL(dec8(GetIYL()));
        return 8;
    case 0x2E: // LD IYL, n
        SetIYL(ReadImmediateByte());
        return 11;
    case 0x34: // INC (IY+d)
        return executeIncDecIndexedIY(true);
    case 0x35: // DEC (IY+d)
        return executeIncDecIndexedIY(false);
    case 0x36: // LD (IY+d), n
        {
            int8_t displacement = ReadDisplacement();
            uint8_t value = ReadImmediateByte();
            uint16_t addr = uint16_t(int32_t(IY) + int32_t(displacement));
            memory->WriteByte(addr, value);
            MEMPTR = addr;
        }
        return 19;
    case 0x39: // ADD IY, SP
        {
            uint16_t oldIY = IY;
            uint16_t result = add16IY(IY, SP);
            MEMPTR = oldIY + 1;
            IY = result;
        }
        return 15;

    // Load register from IY register
    case 0x44: // LD B, IYH
        B = GetIYH();
        return 8;
    case 0x45: // LD B, IYL
        B = GetIYL();
        return 8;
    case 0x46: // LD B, (IY+d)
        return executeLoadFromIndexedIY(0);
    case 0x4C: // LD C, IYH
        C = GetIYH();
        return 8;
    case 0x4D: // LD C, IYL
        C = GetIYL();
        return 8;
    case 0x4E: // LD C, (IY+d)
        return executeLoadFromIndexedIY(1);
    case 0x54: // LD D, IYH
        D = GetIYH();
        return 8;
    case 0x55: // LD D, IYL
        D = GetIYL();
        return 8;
    case 0x56: // LD D, (IY+d)
        return executeLoadFromIndexedIY(2);
    case 0x5C: // LD E, IYH
        E = GetIYH();
        return 8;
    case 0x5D: // LD E, IYL
        E = GetIYL();
        return 8;
    case 0x5E: // LD E, (IY+d)
        return executeLoadFromIndexedIY(3);
    case 0x60: // LD IYH, B
        SetIYH(B);
        return 8;
    case 0x61: // LD IYH, C
        SetIYH(C);
        return 8;
    case 0x62: // LD IYH, D
        SetIYH(D);
        return 8;
    case 0x63: // LD IYH, E
        SetIYH(E);
        return 8;
    case 0x64: // LD IYH, IYH
        // No operation needed
        return 8;
    case 0x65: // LD IYH, IYL
        SetIYH(GetIYL());
        return 8;
    case 0x66: // LD H, (IY+d)
        return executeLoadFromIndexedIY(4);
    case 0x67: // LD IYH, A
        SetIYH(A);
        return 8;
    case 0x68: // LD IYL, B
        SetIYL(B);
        return 8;
    case 0x69: // LD IYL, C
        SetIYL(C);
        return 8;
    case 0x6A: // LD IYL, D
        SetIYL(D);
        return 8;
    case 0x6B: // LD IYL, E
        SetIYL(E);
        return 8;
    case 0x6C: // LD IYL, IYH
        SetIYL(GetIYH());
        return 8;
    case 0x6D: // LD IYL, IYL
        // No operation needed
        return 8;
    case 0x6E: // LD L, (IY+d)
        return executeLoadFromIndexedIY(5);
    case 0x6F: // LD IYL, A
        SetIYL(A);
        return 8;
    case 0x70: // LD (IY+d), B
        return executeStoreToIndexedIY(B);
    case 0x71: // LD (IY+d), C
        return executeStoreToIndexedIY(C);
    case 0x72: // LD (IY+d), D
        return executeStoreToIndexedIY(D);
    case 0x73: // LD (IY+d), E
        return executeStoreToIndexedIY(E);
    case 0x74: // LD (IY+d), H
        return executeStoreToIndexedIY(H);
    case 0x75: // LD (IY+d), L
        return executeStoreToIndexedIY(L);
    case 0x77: // LD (IY+d), A
        return executeStoreToIndexedIY(A);
    case 0x7C: // LD A, IYH
        A = GetIYH();
        return 8;
    case 0x7D: // LD A, IYL
        A = GetIYL();
        return 8;
    case 0x7E: // LD A, (IY+d)
        return executeLoadFromIndexedIY(7);

    // Arithmetic and logic instructions
    case 0x84: // ADD A, IYH
        add8(GetIYH());
        return 8;
    case 0x85: // ADD A, IYL
        add8(GetIYL());
        return 8;
    case 0x86: // ADD A, (IY+d)
        return executeALUIndexedIY(0);
    case 0x8C: // ADC A, IYH
        adc8(GetIYH());
        return 8;
    case 0x8D: // ADC A, IYL
        adc8(GetIYL());
        return 8;
    case 0x8E: // ADC A, (IY+d)
        return executeALUIndexedIY(1);
    case 0x94: // SUB IYH
        sub8(GetIYH());
        return 8;
    case 0x95: // SUB IYL
        sub8(GetIYL());
        return 8;
    case 0x96: // SUB (IY+d)
        return executeALUIndexedIY(2);
    case 0x9C: // SBC A, IYH
        sbc8(GetIYH());
        return 8;
    case 0x9D: // SBC A, IYL
        sbc8(GetIYL());
        return 8;
    case 0x9E: // SBC A, (IY+d)
        return executeALUIndexedIY(3);
    case 0xA4: // AND IYH
        and8(GetIYH());
        return 8;
    case 0xA5: // AND IYL
        and8(GetIYL());
        return 8;
    case 0xA6: // AND (IY+d)
        return executeALUIndexedIY(4);
    case 0xAC: // XOR IYH
        xor8(GetIYH());
        return 8;
    case 0xAD: // XOR IYL
        xor8(GetIYL());
        return 8;
    case 0xAE: // XOR (IY+d)
        return executeALUIndexedIY(5);
    case 0xB4: // OR IYH
        or8(GetIYH());
        return 8;
    case 0xB5: // OR IYL
        or8(GetIYL());
        return 8;
    case 0xB6: // OR (IY+d)
        return executeALUIndexedIY(6);
    case 0xBC: // CP IYH
        cp8(GetIYH());
        return 8;
    case 0xBD: // CP IYL
        cp8(GetIYL());
        return 8;
    case 0xBE: // CP (IY+d)
        return executeALUIndexedIY(7);

    // POP and PUSH instructions
    case 0xE1: // POP IY
        IY = Pop();
        return 14;
    case 0xE3: // EX (SP), IY
        {
            uint16_t temp = memory->ReadWord(SP);
            memory->WriteWord(SP, IY);
            IY = temp;
            MEMPTR = IY;
        }
        return 23;
    case 0xE5: // PUSH IY
        Push(IY);
        return 15;
    case 0xE9: // JP (IY)
        PC = IY;
        return 8;
    case 0xF9: // LD SP, IY
        SP = IY;
        return 10;

    // Handle FD CB prefix (IY with displacement and CB operations)
    case 0xCB: // FD CB prefix
        return ExecuteFDCBOpcode();

    case 0x00: // Extended NOP (undocumented)
        // FD 00 is an undocumented instruction that acts as an extended NOP
        // It consumes the FD prefix and the 00 opcode but executes as a NOP
        // Takes 8 cycles total (4 for FD prefix fetch + 4 for 00 opcode fetch)
        return 8;
    default:
        PC--;
        // Unimplemented opcode - treat as regular opcode
        // This handles cases where FD is followed by a normal opcode
        return ExecuteOpcode();
    }
}

// add16IY adds two 16-bit values for IY register and updates flags
uint16_t Z80::add16IY(uint16_t a, uint16_t b) {
    uint32_t result = (uint32_t)a + (uint32_t)b;
    SetFlag(FLAG_C, result > 0xFFFF);
    SetFlag(FLAG_H, (a & 0x0FFF) + (b & 0x0FFF) > 0x0FFF);
    ClearFlag(FLAG_N);
    // For IY operations, we update X and Y flags from high byte of result
    UpdateFlags3and5FromAddress((uint16_t)result);
    return (uint16_t)result;
}

// Get IYH (high byte of IY)
uint8_t Z80::GetIYH() {
    return uint8_t(IY >> 8);
}

// Get IYL (low byte of IY)
uint8_t Z80::GetIYL() {
    return uint8_t(IY & 0xFF);
}

// Set IYH (high byte of IY)
void Z80::SetIYH(uint8_t value) {
    IY = (IY & 0x00FF) | (uint16_t(value) << 8);
}

// Set IYL (low byte of IY)
void Z80::SetIYL(uint8_t value) {
    IY = (IY & 0xFF00) | uint16_t(value);
}

// executeIncDecIndexedIY handles INC/DEC (IY+d) instructions
int Z80::executeIncDecIndexedIY(bool isInc) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IY) + int32_t(displacement));
    uint8_t value = memory->memory[addr];
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

// executeLoadFromIndexedIY handles LD r, (IY+d) instructions
int Z80::executeLoadFromIndexedIY(uint8_t reg) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IY) + int32_t(displacement));
    uint8_t value = memory->memory[addr];

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

// executeStoreToIndexedIY handles LD (IY+d), r instructions
int Z80::executeStoreToIndexedIY(uint8_t value) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IY) + int32_t(displacement));
    memory->WriteByte(addr, value);
    MEMPTR = addr;
    return 19;
}

// executeALUIndexedIY handles ALU operations with (IY+d) operand
int Z80::executeALUIndexedIY(uint8_t opType) {
    int8_t displacement = ReadDisplacement();
    uint16_t addr = uint16_t(int32_t(IY) + int32_t(displacement));
    uint8_t value = memory->memory[addr];

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
