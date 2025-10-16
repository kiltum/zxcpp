#include "z80.hpp"
#include "memory.hpp"

// Implementation of DD prefixed Z80 opcodes (IX instructions)
int Z80::ExecuteDDOpcode() {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = ReadOpcode();
    
    switch (opcode) {
    // Load instructions
    case 0x09: // ADD IX, BC
        {
            uint16_t oldIX = IX;
            uint16_t result = add16IX(IX, BC);
            MEMPTR = oldIX + 1;
            IX = result;
        }
        return 15;
    case 0x19: // ADD IX, DE
        {
            uint16_t oldIX = IX;
            uint16_t result = add16IX(IX, DE);
            MEMPTR = oldIX + 1;
            IX = result;
        }
        return 15;
    case 0x21: // LD IX, nn
        IX = ReadImmediateWord();
        return 14;
    case 0x22: // LD (nn), IX
        {
            uint16_t addr = ReadImmediateWord();
            memory->WriteWord(addr, IX);
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x23: // INC IX
        IX++;
        return 10;
    case 0x24: // INC IXH
        SetIXH(inc8(GetIXH()));
        return 8;
    case 0x25: // DEC IXH
        SetIXH(dec8(GetIXH()));
        return 8;
    case 0x26: // LD IXH, n
        SetIXH(ReadImmediateByte());
        return 11;
    case 0x29: // ADD IX, IX
        {
            uint16_t oldIX = IX;
            uint16_t result = add16IX(IX, IX);
            MEMPTR = oldIX + 1;
            IX = result;
        }
        return 15;
    case 0x2A: // LD IX, (nn)
        {
            uint16_t addr = ReadImmediateWord();
            IX = memory->ReadWord(addr);
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x2B: // DEC IX
        IX--;
        return 10;
    case 0x2C: // INC IXL
        SetIXL(inc8(GetIXL()));
        return 8;
    case 0x2D: // DEC IXL
        SetIXL(dec8(GetIXL()));
        return 8;
    case 0x2E: // LD IXL, n
        SetIXL(ReadImmediateByte());
        return 11;
    case 0x34: // INC (IX+d)
        return executeIncDecIndexed(true);
    case 0x35: // DEC (IX+d)
        return executeIncDecIndexed(false);
    case 0x36: // LD (IX+d), n
        {
            int8_t displacement = ReadDisplacement();
            uint8_t value = ReadImmediateByte();
            uint16_t addr = uint16_t(int32_t(IX) + int32_t(displacement));
            memory->WriteByte(addr, value);
            MEMPTR = addr;
        }
        return 19;
    case 0x39: // ADD IX, SP
        {
            uint16_t oldIX = IX;
            uint16_t result = add16IX(IX, SP);
            MEMPTR = oldIX + 1;
            IX = result;
        }
        return 15;
    case 0x40: // LD B,B
        return 8;

    // Load register from IX register
    case 0x44: // LD B, IXH
        B = GetIXH();
        return 8;
    case 0x45: // LD B, IXL
        B = GetIXL();
        return 8;
    case 0x46: // LD B, (IX+d)
        return executeLoadFromIndexed(0);
    case 0x4C: // LD C, IXH
        C = GetIXH();
        return 8;
    case 0x4D: // LD C, IXL
        C = GetIXL();
        return 8;
    case 0x4E: // LD C, (IX+d)
        return executeLoadFromIndexed(1);
    case 0x54: // LD D, IXH
        D = GetIXH();
        return 8;
    case 0x55: // LD D, IXL
        D = GetIXL();
        return 8;
    case 0x56: // LD D, (IX+d)
        return executeLoadFromIndexed(2);
    case 0x5C: // LD E, IXH
        E = GetIXH();
        return 8;
    case 0x5D: // LD E, IXL
        E = GetIXL();
        return 8;
    case 0x5E: // LD E, (IX+d)
        return executeLoadFromIndexed(3);
    case 0x60: // LD IXH, B
        SetIXH(B);
        return 8;
    case 0x61: // LD IXH, C
        SetIXH(C);
        return 8;
    case 0x62: // LD IXH, D
        SetIXH(D);
        return 8;
    case 0x63: // LD IXH, E
        SetIXH(E);
        return 8;
    case 0x64: // LD IXH, IXH
        // No operation needed
        return 8;
    case 0x65: // LD IXH, IXL
        SetIXH(GetIXL());
        return 8;
    case 0x66: // LD H, (IX+d)
        return executeLoadFromIndexed(4);
    case 0x67: // LD IXH, A
        SetIXH(A);
        return 8;
    case 0x68: // LD IXL, B
        SetIXL(B);
        return 8;
    case 0x69: // LD IXL, C
        SetIXL(C);
        return 8;
    case 0x6A: // LD IXL, D
        SetIXL(D);
        return 8;
    case 0x6B: // LD IXL, E
        SetIXL(E);
        return 8;
    case 0x6C: // LD IXL, IXH
        SetIXL(GetIXH());
        return 8;
    case 0x6D: // LD IXL, IXL
        // No operation needed
        return 8;
    case 0x6E: // LD L, (IX+d)
        return executeLoadFromIndexed(5);
    case 0x6F: // LD IXL, A
        SetIXL(A);
        return 8;
    case 0x70: // LD (IX+d), B
        return executeStoreToIndexed(B);
    case 0x71: // LD (IX+d), C
        return executeStoreToIndexed(C);
    case 0x72: // LD (IX+d), D
        return executeStoreToIndexed(D);
    case 0x73: // LD (IX+d), E
        return executeStoreToIndexed(E);
    case 0x74: // LD (IX+d), H
        return executeStoreToIndexed(H);
    case 0x75: // LD (IX+d), L
        return executeStoreToIndexed(L);
    case 0x77: // LD (IX+d), A
        return executeStoreToIndexed(A);
    case 0x7C: // LD A, IXH
        A = GetIXH();
        return 8;
    case 0x7D: // LD A, IXL
        A = GetIXL();
        return 8;
    case 0x7E: // LD A, (IX+d)
        return executeLoadFromIndexed(7);

    // Arithmetic and logic instructions
    case 0x84: // ADD A, IXH
        add8(GetIXH());
        return 8;
    case 0x85: // ADD A, IXL
        add8(GetIXL());
        return 8;
    case 0x86: // ADD A, (IX+d)
        return executeALUIndexed(0);
    case 0x8C: // ADC A, IXH
        adc8(GetIXH());
        return 8;
    case 0x8D: // ADC A, IXL
        adc8(GetIXL());
        return 8;
    case 0x8E: // ADC A, (IX+d)
        return executeALUIndexed(1);
    case 0x94: // SUB IXH
        sub8(GetIXH());
        return 8;
    case 0x95: // SUB IXL
        sub8(GetIXL());
        return 8;
    case 0x96: // SUB (IX+d)
        return executeALUIndexed(2);
    case 0x9C: // SBC A, IXH
        sbc8(GetIXH());
        return 8;
    case 0x9D: // SBC A, IXL
        sbc8(GetIXL());
        return 8;
    case 0x9E: // SBC A, (IX+d)
        return executeALUIndexed(3);
    case 0xA4: // AND IXH
        and8(GetIXH());
        return 8;
    case 0xA5: // AND IXL
        and8(GetIXL());
        return 8;
    case 0xA6: // AND (IX+d)
        return executeALUIndexed(4);
    case 0xAC: // XOR IXH
        xor8(GetIXH());
        return 8;
    case 0xAD: // XOR IXL
        xor8(GetIXL());
        return 8;
    case 0xAE: // XOR (IX+d)
        return executeALUIndexed(5);
    case 0xB4: // OR IXH
        or8(GetIXH());
        return 8;
    case 0xB5: // OR IXL
        or8(GetIXL());
        return 8;
    case 0xB6: // OR (IX+d)
        return executeALUIndexed(6);
    case 0xBC: // CP IXH
        cp8(GetIXH());
        return 8;
    case 0xBD: // CP IXL
        cp8(GetIXL());
        return 8;
    case 0xBE: // CP (IX+d)
        return executeALUIndexed(7);

    // POP and PUSH instructions
    case 0xE1: // POP IX
        IX = Pop();
        return 14;
    case 0xE3: // EX (SP), IX
        {
            uint16_t temp = memory->ReadWord(SP);
            memory->WriteWord(SP, IX);
            IX = temp;
            MEMPTR = temp;
        }
        return 23;
    case 0xE5: // PUSH IX
        Push(IX);
        return 15;
    case 0xE9: // JP (IX)
        PC = IX;
        return 8;
    case 0xF9: // LD SP, IX
        SP = IX;
        return 10;

    // Handle DD CB prefix (IX with displacement and CB operations)
    case 0xCB: // DD CB prefix
        return executeDDCBOpcode();

    case 0xfd:
        return 8;
    case 0x00: // Extended NOP (undocumented)
        // DD 00 is an undocumented instruction that acts as an extended NOP
        // It consumes the DD prefix and the 00 opcode but executes as a NOP
        // Takes 8 cycles total (4 for DD prefix fetch + 4 for 00 opcode fetch)
        return 8;
    case 0xdd:
        return 8;
    default:
        return ExecuteOpcode();
        //panic(fmt.Sprintf("DD unexpected code %x", opcode))
    }
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
