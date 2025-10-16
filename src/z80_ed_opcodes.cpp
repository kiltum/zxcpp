#include "z80.hpp"
#include "memory.hpp"

// Implementation of ED prefixed Z80 opcodes (extended instructions)
int Z80::ExecuteEDOpcode() {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = ReadOpcode();
    
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
        //R = (R & 0x80) | (A & 0x7F);
        R = A; // fix zen80 tests
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
