#include "z80.hpp"
#include "memory.hpp"

// Implementation of ED prefixed Z80 opcodes (extended instructions)
int Z80::ExecuteEDOpcode() {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = ReadOpcode();
    R++;
    switch (opcode) {
    // Block transfer instructions
    case 0xA0: // LDI
        ldi();
        return 16;
    case 0xA1: // CPI
        cpi();
        return 16;
    case 0xA2: // INI
        ini();
        return 16;
    case 0xA3: // OUTI
        outi();
        return 16;
    case 0xA8: // LDD
        ldd();
        return 16;
    case 0xA9: // CPD
        cpd();
        return 16;
    case 0xAA: // IND
        ind();
        return 16;
    case 0xAB: // OUTD
        outd();
        return 16;
    case 0xB0: // LDIR
        return ldir();
    case 0xB1: // CPIR
        return cpir();
    case 0xB2: // INIR
        return inir();
    case 0xB3: // OTIR
        return otir();
    case 0xB8: // LDDR
        return lddr();
    case 0xB9: // CPDR
        return cpdr();
    case 0xBA: // INDR
        return indr();
    case 0xBB: // OTDR
        return otdr();

    // 8-bit load instructions
    case 0x40: // IN B, (C)
        return executeIN(0);
    case 0x41: // OUT (C), B
        return executeOUT(0);
    case 0x42: // SBC HL, BC
        {
            uint16_t result = sbc16WithMEMPTR(HL, BC);
            HL = result;
        }
        return 15;
    case 0x43: // LD (nn), BC
        {
            uint16_t addr = ReadImmediateWord();
            memory->WriteWord(addr, BC);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x44: // NEG (various undocumented versions)
    case 0x4C:
    case 0x54:
    case 0x5C:
    case 0x64:
    case 0x6C:
    case 0x74:
    case 0x7C:
        neg();
        return 8;
    case 0x45: // RETN (various undocumented versions)
    case 0x55:
    case 0x5D:
    case 0x65:
    case 0x6D:
    case 0x75:
    case 0x7D:
        retn();
        return 14;
    case 0x46: // IM 0 (various undocumented versions)
    case 0x4E:
    case 0x66:
        IM = 0;
        return 8;
    case 0x47: // LD I, A
        I = A;
        return 9;
    case 0x48: // IN C, (C)
        return executeIN(1);
    case 0x49: // OUT (C), C
        return executeOUT(1);
    case 0x4A: // ADC HL, BC
        {
            uint16_t result = adc16WithMEMPTR(HL, BC);
            HL = result;
        }
        return 15;
    case 0x4B: // LD BC, (nn)
        {
            uint16_t addr = ReadImmediateWord();
            BC = memory->ReadWord(addr);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x4D: // RETI
        reti();
        return 14;
    case 0x4F: // LD R, A
        // R register is only 7 bits, bit 7 remains unchanged
        R = (R & 0x80) | (A & 0x7F);
        //R = A; // fix zen80 tests
        return 9;
    case 0x50: // IN D, (C)
        return executeIN(2);
    case 0x51: // OUT (C), D
        return executeOUT(2);
    case 0x52: // SBC HL, DE
        {
            uint16_t result = sbc16WithMEMPTR(HL, DE);
            HL = result;
        }
        return 15;
    case 0x53: // LD (nn), DE
        {
            uint16_t addr = ReadImmediateWord();
            memory->WriteWord(addr, DE);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x56: // IM 1 (various undocumented versions)
    case 0x76:
        IM = 1;
        return 8;
    case 0x57: // LD A, I
        ldAI();
        return 9;
    case 0x58: // IN E, (C)
        return executeIN(3);
    case 0x59: // OUT (C), E
        return executeOUT(3);
    case 0x5A: // ADC HL, DE
        {
            uint16_t result = adc16WithMEMPTR(HL, DE);
            HL = result;
        }
        return 15;
    case 0x5B: // LD DE, (nn)
        {
            uint16_t addr = ReadImmediateWord();
            DE = memory->ReadWord(addr);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x5E: // IM 2 (various undocumented versions)
    case 0x7E:
        IM = 2;
        return 8;
    case 0x5F: // LD A, R
        ldAR();
        return 9;
    case 0x60: // IN H, (C)
        return executeIN(4);
    case 0x61: // OUT (C), H
        return executeOUT(4);
    case 0x62: // SBC HL, HL
        {
            uint16_t result = sbc16WithMEMPTR(HL, HL);
            HL = result;
        }
        return 15;
    case 0x63: // LD (nn), HL
        {
            uint16_t addr = ReadImmediateWord();
            memory->WriteWord(addr, HL);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x67: // RRD
        rrd();
        return 18;
    case 0x68: // IN L, (C)
        return executeIN(5);
    case 0x69: // OUT (C), L
        return executeOUT(5);
    case 0x6A: // ADC HL, HL
        {
            uint16_t result = adc16WithMEMPTR(HL, HL);
            HL = result;
        }
        return 15;
    case 0x6B: // LD HL, (nn)
        {
            uint16_t addr = ReadImmediateWord();
            HL = memory->ReadWord(addr);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x6F: // RLD
        rld();
        return 18;
    case 0x70: // IN (C) (Undocumented - input to dummy register)
        {
            uint16_t bc = BC; // Save BC before doing anything
            uint8_t value = inC();
            UpdateSZXYFlags(value);
            ClearFlag(FLAG_H);
            ClearFlag(FLAG_N);
            // Set PV flag based on parity of the result
            SetFlag(FLAG_PV, parity(value));
            // MEMPTR = BC + 1 (using the original BC value)
            MEMPTR = bc + 1;
        }
        return 12;
    case 0x71: // OUT (C), 0 (Undocumented)
        {
            outC(0);
            // MEMPTR = BC + 1
            MEMPTR = BC + 1;
        }
        return 12;
    case 0x72: // SBC HL, SP
        {
            uint16_t result = sbc16WithMEMPTR(HL, SP);
            HL = result;
        }
        return 15;
    case 0x73: // LD (nn), SP
        {
            uint16_t addr = ReadImmediateWord();
            memory->WriteWord(addr, SP);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x78: // IN A, (C)
        return executeIN(7);
    case 0x79: // OUT (C), A
        return executeOUT(7);
    case 0x7A: // ADC HL, SP
        {
            uint16_t result = adc16WithMEMPTR(HL, SP);
            HL = result;
        }
        return 15;
    case 0x7B: // LD SP, (nn)
        {
            uint16_t addr = ReadImmediateWord();
            SP = memory->ReadWord(addr);
            // MEMPTR = addr + 1
            MEMPTR = addr + 1;
        }
        return 20;
    case 0x80: // undefined NOP
        return 8;
    case 0x6e:
        return 8;

    default:
        // For unimplemented opcodes, we just return a default cycle count
        return 4;
    }
}

// sbc16 subtracts 16-bit value with carry from HL
uint16_t Z80::sbc16(uint16_t val1, uint16_t val2) {
    uint32_t carry = 0;
    if (GetFlag(FLAG_C)) {
        carry = 1;
    }
    int32_t result = (int32_t)val1 - (int32_t)val2 - (int32_t)carry;
    bool halfCarry = ((int16_t)(val1 & 0x0FFF) - (int16_t)(val2 & 0x0FFF) - (int16_t)carry) < 0;
    bool overflow = (((val1 ^ val2) & 0x8000) != 0) && (((val1 ^ (uint16_t)result) & 0x8000) != 0);

    uint16_t res16 = (uint16_t)result;

    SetFlag(FLAG_S, (res16 & 0x8000) != 0);
    SetFlag(FLAG_Z, res16 == 0);
    SetFlag(FLAG_H, halfCarry);
    SetFlag(FLAG_PV, overflow);
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_C, result < 0);
    // FIX: Set X and Y flags from high byte of result
    SetFlag(FLAG_X, (uint8_t(res16 >> 8) & FLAG_X) != 0);
    SetFlag(FLAG_Y, (uint8_t(res16 >> 8) & FLAG_Y) != 0);
    MEMPTR = val1 + 1;
    return res16;
}

// sbc16WithMEMPTR subtracts 16-bit value with carry from HL and sets MEMPTR
uint16_t Z80::sbc16WithMEMPTR(uint16_t a, uint16_t b) {
    uint16_t result = sbc16(a, b);
    MEMPTR = a + 1;
    return result;
}

// adc16 adds 16-bit value with carry to HL
uint16_t Z80::adc16(uint16_t val1, uint16_t val2) {
    uint32_t carry = 0;
    if (GetFlag(FLAG_C)) {
        carry = 1;
    }
    uint32_t result = (uint32_t)val1 + (uint32_t)val2 + carry;
    bool halfCarry = ((val1 & 0x0FFF) + (val2 & 0x0FFF) + (uint16_t)carry) > 0x0FFF;
    bool overflow = (((val1 ^ val2) & 0x8000) == 0) && (((val1 ^ (uint16_t)result) & 0x8000) != 0);

    uint16_t res16 = (uint16_t)result;

    SetFlag(FLAG_S, (res16 & 0x8000) != 0);
    SetFlag(FLAG_Z, res16 == 0);
    SetFlag(FLAG_H, halfCarry);
    SetFlag(FLAG_PV, overflow);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, result > 0xFFFF);
    // FIX: Set X and Y flags from high byte of result
    SetFlag(FLAG_X, (uint8_t(res16 >> 8) & FLAG_X) != 0);
    SetFlag(FLAG_Y, (uint8_t(res16 >> 8) & FLAG_Y) != 0);
    MEMPTR = val1 + 1;

    return res16;
}

// adc16WithMEMPTR adds 16-bit value with carry to HL and sets MEMPTR
uint16_t Z80::adc16WithMEMPTR(uint16_t a, uint16_t b) {
    uint16_t result = adc16(a, b);
    MEMPTR = a + 1;
    return result;
}

// neg negates the accumulator
void Z80::neg() {
    uint8_t value = A;
    A = 0;
    sub8(value);
}

// retn returns from interrupt and restores IFF1 from IFF2
void Z80::retn() {
    PC = Pop();
    MEMPTR = PC;
    IFF1 = IFF2;
}

// reti returns from interrupt (same as retn for Z80)
void Z80::reti() {
    PC = Pop();
    MEMPTR = PC;
    IFF1 = IFF2;
}

// ldAI loads I register into A and updates flags
void Z80::ldAI() {
    A = I;
    UpdateSZXYFlags(A);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    SetFlag(FLAG_PV, IFF2);
}

// ldAR loads R register into A and updates flags
void Z80::ldAR() {
    // Load the R register into A
    A = R;
    UpdateSZXYFlags(A);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    SetFlag(FLAG_PV, IFF2);
}

// rrd rotates digit between A and (HL) right
void Z80::rrd() {
    uint8_t value = memory->ReadByte(HL);
    uint8_t ah = A & 0xF0;
    uint8_t al = A & 0x0F;
    uint8_t hl = value;

    // A bits 3-0 go to HL bits 7-4
    // HL bits 7-4 go to HL bits 3-0
    // HL bits 3-0 go to A bits 3-0
    A = ah | (hl & 0x0F);
    uint8_t newHL = ((hl & 0xF0) >> 4) | (al << 4);
    memory->WriteByte(HL, newHL);

    UpdateSZXYPVFlags(A);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);

    // Set MEMPTR = HL + 1
    MEMPTR = HL + 1;
}

// rld rotates digit between A and (HL) left
void Z80::rld() {
    uint8_t value = memory->ReadByte(HL);
    uint8_t ah = A & 0xF0;
    uint8_t al = A & 0x0F;
    uint8_t hl = value;

    // A bits 3-0 go to HL bits 3-0
    // HL bits 3-0 go to HL bits 7-4
    // HL bits 7-4 go to A bits 3-0
    A = ah | (hl >> 4);
    uint8_t newHL = ((hl & 0x0F) << 4) | al;
    memory->WriteByte(HL, newHL);

    UpdateSZXYPVFlags(A);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);

    // Set MEMPTR = HL + 1
    MEMPTR = HL + 1;
}

// executeIN handles the IN r, (C) instructions
int Z80::executeIN(uint8_t reg) {
    uint16_t bc = BC; // Save BC before doing anything
    uint8_t value = inC();

    // Update flags
    UpdateSZXYFlags(value);
    ClearFlag(FLAG_H);
    ClearFlag(FLAG_N);
    // Set PV flag based on parity of the result
    SetFlag(FLAG_PV, parity(value));
    // MEMPTR = BC + 1 (using the original BC value)
    MEMPTR = bc + 1;

    // Set the appropriate register
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

    return 12;
}

// executeOUT handles the OUT (C), r instructions
int Z80::executeOUT(uint8_t reg) {
    uint8_t value;

    // Get the appropriate register value
    switch (reg) {
    case 0:
        value = B;
        break;
    case 1:
        value = C;
        break;
    case 2:
        value = D;
        break;
    case 3:
        value = E;
        break;
    case 4:
        value = H;
        break;
    case 5:
        value = L;
        break;
    case 7:
        value = A;
        break;
    default:
        value = 0;
        break;
    }

    outC(value);
    // MEMPTR = BC + 1
    MEMPTR = BC + 1;

    return 12;
}

// ldi loads byte from (HL) to (DE), increments pointers, decrements BC
void Z80::ldi() {
    uint8_t value = memory->ReadByte(HL);
    memory->WriteByte(DE, value);

    DE++;
    HL++;
    BC--;

    // FIXED: Calculate X and Y flags FIRST, preserving S, Z, C
    uint8_t n = value + A;
    F = (F & (FLAG_S | FLAG_Z | FLAG_C)) | (n & FLAG_X) | ((n & 0x02) << 4);

    // THEN set the other flags
    ClearFlag(FLAG_H);
    SetFlag(FLAG_PV, BC != 0);
    ClearFlag(FLAG_N);
}

// cpi compares A with (HL), increments HL, decrements BC
void Z80::cpi() {
    uint8_t value = memory->ReadByte(HL);
    uint8_t result = A - value;

    HL++;
    BC--;

    SetFlag(FLAG_N, true);
    UpdateSZFlags(result);

    // Set H flag if borrow from bit 4
    SetFlag(FLAG_H, (A & 0x0F) < (value & 0x0F));

    // For CPI, F3 and F5 flags come from (A - (HL) - H_flag)
    // where H_flag is the half-carry flag AFTER the instruction
    uint8_t temp = result - (GetFlag(FLAG_H) ? 1 : 0);
    SetFlag(FLAG_X, (temp & 0x08) != 0); // Bit 3
    SetFlag(FLAG_Y, (temp & 0x02) != 0); // Bit 1

    if (BC != 0) {
        SetFlag(FLAG_PV, true);
    } else {
        ClearFlag(FLAG_PV);
    }

    // Set MEMPTR = PC - 1
    MEMPTR = PC - 1;
}

// ini inputs byte to (HL), increments HL, decrements B
void Z80::ini() {
    uint8_t value = port->Read(uint16_t(C) | (uint16_t(B) << 8));
    memory->WriteByte(HL, value);
    HL++;
    uint16_t origbc = BC;
    B--;

    // Enhanced: Accurate flag calculation for INI
    int k = (int)value + (int)((C + 1) & 0xFF);

    SetFlag(FLAG_Z, B == 0);
    SetFlag(FLAG_S, (B & 0x80) != 0);
    SetFlag(FLAG_N, (value & 0x80) != 0);
    SetFlag(FLAG_H, k > 0xFF);
    SetFlag(FLAG_C, k > 0xFF);
    // P/V flag is parity of ((k & 0x07) XOR B)
    SetFlag(FLAG_PV, parity(uint8_t(k & 0x07) ^ B));
    // X and Y flags from B register
    F = (F & 0xD7) | (B & (FLAG_X | FLAG_Y));

    MEMPTR = origbc + 1;
}

// outi outputs byte from (HL) to port, increments HL, decrements B
void Z80::outi() {
    uint8_t val = memory->ReadByte(HL);
    B--;
    port->Write(BC, val);
    HL++;

    // Enhanced: Accurate flag calculation for OUTI
    // Note: Use L after HL increment
    int k = (int)val + (int)L;

    SetFlag(FLAG_Z, B == 0);
    SetFlag(FLAG_S, (B & 0x80) != 0);
    SetFlag(FLAG_N, (val & 0x80) != 0);
    SetFlag(FLAG_H, k > 0xFF);
    SetFlag(FLAG_C, k > 0xFF);
    // P/V flag is parity of ((k & 0x07) XOR B)
    uint8_t pvVal = uint8_t(k & 0x07) ^ B;
    SetFlag(FLAG_PV, parity(pvVal));
    // X and Y flags from B register
    F = (F & 0xD7) | (B & (FLAG_X | FLAG_Y));

    MEMPTR = BC + 1;
}

// ldd loads byte from (HL) to (DE), decrements pointers, decrements BC
void Z80::ldd() {
    uint8_t value = memory->ReadByte(HL);
    memory->WriteByte(DE, value);
    HL--;
    DE--;
    BC--;

    // FIXED: Calculate X and Y flags FIRST, preserving S, Z, C
    uint8_t n = value + A;
    F = (F & (FLAG_S | FLAG_Z | FLAG_C)) | (n & FLAG_X) | ((n & 0x02) << 4);

    // THEN set the other flags
    ClearFlag(FLAG_H);
    SetFlag(FLAG_PV, BC != 0);
    ClearFlag(FLAG_N);
}

// cpd compares A with (HL), decrements HL, decrements BC
void Z80::cpd() {
    uint8_t val = memory->ReadByte(HL);
    int16_t result = (int16_t)A - (int16_t)val;
    HL--;
    BC--;

    SetFlag(FLAG_S, (uint8_t(result) & 0x80) != 0);
    SetFlag(FLAG_Z, uint8_t(result) == 0);
    SetFlag(FLAG_H, (int8_t(A & 0x0F) - int8_t(val & 0x0F)) < 0);
    SetFlag(FLAG_PV, BC != 0);
    SetFlag(FLAG_N, true);

    // Y flag calculation - preserve S, Z, H, PV, N, C flags
    uint8_t n = uint8_t(result);
    if (GetFlag(FLAG_H)) {
        n--;
    }
    F = (F & (FLAG_S | FLAG_Z | FLAG_H | FLAG_PV | FLAG_N | FLAG_C)) | (n & FLAG_X) | ((n & 0x02) << 4);
    MEMPTR--;
}

// ind inputs byte to (HL), decrements HL, decrements B
void Z80::ind() {
    uint8_t val = port->Read(BC);
    memory->WriteByte(HL, val);
    HL--;
    MEMPTR = BC - 1;
    B--;

    // Enhanced: Accurate flag calculation for IND
    // Note: Based on Z80 documentation, k = val + C (not C-1)
    // HUMAN: based on fuse test , ITS + C-1
    //k := int(val) + int(cpu.C)

    SetFlag(FLAG_Z, B == 0);
    SetFlag(FLAG_S, (B & 0x80) != 0);
    SetFlag(FLAG_N, (val & 0x80) != 0);
    // HUMAN : here was error
    // cpu.SetFlagState(FLAG_H, k > 0xFF)
    // cpu.SetFlagState(FLAG_C, k > 0xFF)
    // // P/V flag is parity of ((k & 0x07) XOR B)
    // pvVal := uint8(k&0x07) ^ cpu.B
    // cpu.SetFlagState(FLAG_PV, parity(pvVal))

    uint16_t diff = uint16_t(C - 1) + uint16_t(val);
    SetFlag(FLAG_H, diff > 0xFF);
    SetFlag(FLAG_C, diff > 0xFF);
    uint8_t temp = uint8_t((diff & 0x07) ^ uint16_t(B));
    uint8_t parity_val = 0;
    for (int i = 0; i < 8; i++) {
        parity_val ^= (temp >> i) & 1;
    }
    SetFlag(FLAG_PV, parity_val == 0);

    // X and Y flags from B register
    F = (F & 0xD7) | (B & (FLAG_X | FLAG_Y));
}

// outd outputs byte from (HL) to port, decrements HL, decrements B
void Z80::outd() {
    uint8_t val = memory->ReadByte(HL);
    B--;
    port->Write(uint16_t(C) | (uint16_t(B) << 8), val);
    HL--;

    uint16_t k = uint16_t(val) + uint16_t(L);

    SetFlag(FLAG_Z, B == 0);
    SetFlag(FLAG_S, (B & 0x80) != 0);
    SetFlag(FLAG_N, (val & 0x80) != 0);
    SetFlag(FLAG_H, k > 0xFF);
    SetFlag(FLAG_C, k > 0xFF);
    // P/V flag is parity of ((k & 0x07) XOR B)
    uint8_t pvVal = uint8_t(k & 0x07) ^ B;
    SetFlag(FLAG_PV, parity(pvVal));
    // X and Y flags from B register
    F = (F & 0xD7) | (B & (FLAG_X | FLAG_Y));

    MEMPTR = BC - 1;
}

// ldir repeated LDI until BC=0
int Z80::ldir() {
    ldi();

    // Add T-states for this iteration (21 for continuing, 16 for final)
    if (BC != 0) {
        PC -= 2;
        MEMPTR = PC + 1;
        return 21;
    } else {
        return 16;
    }
}

// cpir repeated CPI until BC=0 or A=(HL)
int Z80::cpir() {
    cpi();

    if (BC != 0 && !GetFlag(FLAG_Z)) {
        PC -= 2; // Repeat instruction

        // Return T-states for continuing iteration
        return 21;
    } else {
        // Return T-states for final iteration
        MEMPTR = PC;
        return 16;
    }
}

// inir repeated INI until B=0
int Z80::inir() {
    ini();

    if (B != 0) {
        PC -= 2; // Repeat instruction
        // Return T-states for continuing iteration
        return 21;
    } else {
        // Set MEMPTR to PC+1 at the end of the instruction
        //cpu.MEMPTR = cpu.PC
        // Return T-states for final iteration
        return 16;
    }
}

// otir repeated OUTI until B=0
int Z80::otir() {
    outi();

    if (B != 0) {
        PC -= 2; // Repeat instruction
        // Return T-states for continuing iteration
        return 21;
    } else {
        // Return T-states for final iteration
        return 16;
    }
}

// lddr repeated LDD until BC=0
int Z80::lddr() {
    // Execute one LDD operation
    ldd();

    // Add T-states for this iteration (21 for continuing, 16 for final)
    if (BC != 0) {
        PC -= 2;
        MEMPTR = PC + 1;
        return 21;
    } else {
        return 16;
    }
}

// cpdr repeated CPD until BC=0 or A=(HL)
int Z80::cpdr() {
    cpd();

    if (BC != 0 && !GetFlag(FLAG_Z)) {
        PC -= 2; // Repeat instruction
        // Return T-states for continuing iteration
        MEMPTR = PC + 1;
        return 21;
    } else {
        MEMPTR = PC - 2;
        // Return T-states for final iteration
        return 16;
    }
}

// indr repeated IND until B=0
int Z80::indr() {
    ind();

    if (B != 0) {
        PC -= 2; // Repeat instruction
        // Return T-states for continuing iteration
        return 21;
    } else {
        // Return T-states for final iteration
        return 16;
    }
}

// otdr repeated OUTD until B=0
int Z80::otdr() {
    outd();

    if (B != 0) {
        PC -= 2; // Repeat instruction
        // Return T-states for continuing iteration
        return 21;
    } else {
        return 16;
    }
}

// inC reads from port (BC)
uint8_t Z80::inC() {
    return port->Read(BC);
}

// outC writes to port (BC)
void Z80::outC(uint8_t value) {
    port->Write(BC, value);
}
