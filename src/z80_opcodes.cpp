#include "z80_opcodes.hpp"
#include "z80.hpp"
#include "memory.hpp"
#include "port.hpp"

// Implementation of common Z80 opcodes
int ExecuteOpcode(Z80* cpu) {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = cpu->ReadOpcode();
    
    switch (opcode) {
        // 8-bit load group
        case 0x00: // NOP
            return 4;
        case 0x01: // LD BC, nn
            cpu->BC = cpu->ReadImmediateWord();
            return 10;
        case 0x02: // LD (BC), A
            cpu->memory->WriteByte(cpu->BC, cpu->A);
            cpu->MEMPTR = (uint16_t(cpu->A) << 8) | (uint16_t(cpu->BC + 1) & 0xff);
            return 7;
        case 0x03: // INC BC
            cpu->BC++;
            return 6;
        case 0x04: // INC B
            cpu->B = cpu->inc8(cpu->B);
            return 4;
        case 0x05: // DEC B
            cpu->B = cpu->dec8(cpu->B);
            return 4;
        case 0x06: // LD B, n
            cpu->B = cpu->ReadImmediateByte();
            return 7;
        case 0x07: // RLCA
            cpu->rlca();
            return 4;
        case 0x08: // EX AF, AF'
            {
                uint16_t temp = cpu->AF;
                cpu->AF = cpu->AF_;
                cpu->AF_ = temp;
            }
            return 4;
        case 0x09: // ADD HL, BC
            {
                uint16_t result = cpu->add16(cpu->HL, cpu->BC);
                cpu->MEMPTR = cpu->HL + 1;
                cpu->HL = result;
            }
            return 11;
        case 0x0A: // LD A, (BC)
            cpu->A = cpu->memory->ReadByte(cpu->BC);
            cpu->MEMPTR = cpu->BC + 1;
            return 7;
        case 0x0B: // DEC BC
            cpu->BC--;
            return 6;
        case 0x0C: // INC C
            cpu->C = cpu->inc8(cpu->C);
            return 4;
        case 0x0D: // DEC C
            cpu->C = cpu->dec8(cpu->C);
            return 4;
        case 0x0E: // LD C, n
            cpu->C = cpu->ReadImmediateByte();
            return 7;
        case 0x0F: // RRCA
            cpu->rrca();
            return 4;
        case 0x10: // DJNZ e
            cpu->B--;
            if (cpu->B != 0) {
                int8_t offset = cpu->ReadDisplacement();
                cpu->MEMPTR = cpu->PC + uint16_t(int32_t(offset));
                cpu->PC = uint16_t(int32_t(cpu->PC) + int32_t(offset));
                return 13;
            }
            cpu->PC++; // Skip the offset byte
            return 8;
        case 0x11: // LD DE, nn
            cpu->DE = cpu->ReadImmediateWord();
            return 10;
        case 0x12: // LD (DE), A
            cpu->memory->WriteByte(cpu->DE, cpu->A);
            cpu->MEMPTR = (uint16_t(cpu->A) << 8) | (uint16_t(cpu->DE + 1) & 0xff);
            return 7;
        case 0x13: // INC DE
            cpu->DE++;
            return 6;
        case 0x14: // INC D
            cpu->D = cpu->inc8(cpu->D);
            return 4;
        case 0x15: // DEC D
            cpu->D = cpu->dec8(cpu->D);
            return 4;
        case 0x16: // LD D, n
            cpu->D = cpu->ReadImmediateByte();
            return 7;
        case 0x17: // RLA
            cpu->rla();
            return 4;
        case 0x18: // JR e
            {
                int8_t offset = cpu->ReadDisplacement();
                cpu->MEMPTR = cpu->PC + uint16_t(int32_t(offset));
                cpu->PC = uint16_t(int32_t(cpu->PC) + int32_t(offset));
            }
            return 12;
        case 0x19: // ADD HL, DE
            {
                uint16_t result = cpu->add16(cpu->HL, cpu->DE);
                cpu->MEMPTR = cpu->HL + 1;
                cpu->HL = result;
            }
            return 11;
        case 0x1A: // LD A, (DE)
            cpu->A = cpu->memory->ReadByte(cpu->DE);
            cpu->MEMPTR = cpu->DE + 1;
            return 7;
        case 0x1B: // DEC DE
            cpu->DE--;
            return 6;
        case 0x1C: // INC E
            cpu->E = cpu->inc8(cpu->E);
            return 4;
        case 0x1D: // DEC E
            cpu->E = cpu->dec8(cpu->E);
            return 4;
        case 0x1E: // LD E, n
            cpu->E = cpu->ReadImmediateByte();
            return 7;
        case 0x1F: // RRA
            cpu->rra();
            return 4;
        case 0x20: // JR NZ, e
            if (!cpu->GetFlag(FLAG_Z)) {
                int8_t offset = cpu->ReadDisplacement();
                cpu->MEMPTR = cpu->PC + uint16_t(int32_t(offset));
                cpu->PC = uint16_t(int32_t(cpu->PC) + int32_t(offset));
                return 12;
            }
            cpu->PC++; // Skip the offset byte
            return 7;
        case 0x21: // LD HL, nn
            cpu->HL = cpu->ReadImmediateWord();
            return 10;
        case 0x22: // LD (nn), HL
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->memory->WriteWord(addr, cpu->HL);
                cpu->MEMPTR = addr + 1;
            }
            return 16;
        case 0x23: // INC HL
            cpu->HL++;
            return 6;
        case 0x24: // INC H
            cpu->H = cpu->inc8(cpu->H);
            return 4;
        case 0x25: // DEC H
            cpu->H = cpu->dec8(cpu->H);
            return 4;
        case 0x26: // LD H, n
            cpu->H = cpu->ReadImmediateByte();
            return 7;
        case 0x27: // DAA
            cpu->daa();
            return 4;
        case 0x28: // JR Z, e
            if (cpu->GetFlag(FLAG_Z)) {
                int8_t offset = cpu->ReadDisplacement();
                cpu->MEMPTR = uint16_t(int32_t(cpu->PC) + int32_t(offset));
                cpu->PC = uint16_t(int32_t(cpu->PC) + int32_t(offset));
                return 12;
            }
            cpu->ReadDisplacement(); // Skip the offset byte
            return 7;
        case 0x29: // ADD HL, HL
            {
                uint16_t result = cpu->add16(cpu->HL, cpu->HL);
                cpu->MEMPTR = cpu->HL + 1;
                cpu->HL = result;
            }
            return 11;
        case 0x2A: // LD HL, (nn)
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->HL = cpu->memory->ReadWord(addr);
                cpu->MEMPTR = addr + 1;
            }
            return 16;
        case 0x2B: // DEC HL
            cpu->HL--;
            return 6;
        case 0x2C: // INC L
            cpu->L = cpu->inc8(cpu->L);
            return 4;
        case 0x2D: // DEC L
            cpu->L = cpu->dec8(cpu->L);
            return 4;
        case 0x2E: // LD L, n
            cpu->L = cpu->ReadImmediateByte();
            return 7;
        case 0x2F: // CPL
            cpu->cpl();
            return 4;
        case 0x30: // JR NC, e
            if (!cpu->GetFlag(FLAG_C)) {
                int8_t offset = cpu->ReadDisplacement();
                cpu->MEMPTR = cpu->PC + uint16_t(int32_t(offset));
                cpu->PC = uint16_t(int32_t(cpu->PC) + int32_t(offset));
                return 12;
            }
            cpu->PC++; // Skip the offset byte
            return 7;
        case 0x31: // LD SP, nn
            cpu->SP = cpu->ReadImmediateWord();
            return 10;
        case 0x32: // LD (nn), A
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->memory->WriteByte(addr, cpu->A);
                cpu->MEMPTR = (uint16_t(cpu->A) << 8) | ((addr + 1) & 0xFF);
            }
            return 13;
        case 0x33: // INC SP
            cpu->SP++;
            return 6;
        case 0x34: // INC (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                uint8_t result = cpu->inc8(value);
                cpu->memory->WriteByte(cpu->HL, result);
            }
            return 11;
        case 0x35: // DEC (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                uint8_t result = cpu->dec8(value);
                cpu->memory->WriteByte(cpu->HL, result);
            }
            return 11;
        case 0x36: // LD (HL), n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->memory->WriteByte(cpu->HL, value);
            }
            return 10;
        case 0x37: // SCF
            cpu->scf();
            return 4;
        case 0x38: // JR C, e
            if (cpu->GetFlag(FLAG_C)) {
                int8_t offset = cpu->ReadDisplacement();
                cpu->MEMPTR = cpu->PC + uint16_t(int32_t(offset));
                cpu->PC = uint16_t(int32_t(cpu->PC) + int32_t(offset));
                return 12;
            }
            cpu->PC++; // Skip the offset byte
            return 7;
        case 0x39: // ADD HL, SP
            {
                uint16_t result = cpu->add16(cpu->HL, cpu->SP);
                cpu->MEMPTR = cpu->HL + 1;
                cpu->HL = result;
            }
            return 11;
        case 0x3A: // LD A, (nn)
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->A = cpu->memory->ReadByte(addr);
                cpu->MEMPTR = addr + 1;
            }
            return 13;
        case 0x3B: // DEC SP
            cpu->SP--;
            return 6;
        case 0x3C: // INC A
            cpu->A = cpu->inc8(cpu->A);
            return 4;
        case 0x3D: // DEC A
            cpu->A = cpu->dec8(cpu->A);
            return 4;
        case 0x3E: // LD A, n
            cpu->A = cpu->ReadImmediateByte();
            return 7;
        case 0x3F: // CCF
            cpu->ccf();
            return 4;

        // LD r, r' instructions
        case 0x40: // LD B, B
            return 4;
        case 0x41: // LD B, C
            cpu->B = cpu->C;
            return 4;
        case 0x42: // LD B, D
            cpu->B = cpu->D;
            return 4;
        case 0x43: // LD B, E
            cpu->B = cpu->E;
            return 4;
        case 0x44: // LD B, H
            cpu->B = cpu->H;
            return 4;
        case 0x45: // LD B, L
            cpu->B = cpu->L;
            return 4;
        case 0x46: // LD B, (HL)
            cpu->B = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x47: // LD B, A
            cpu->B = cpu->A;
            return 4;
        case 0x48: // LD C, B
            cpu->C = cpu->B;
            return 4;
        case 0x49: // LD C, C
            return 4;
        case 0x4A: // LD C, D
            cpu->C = cpu->D;
            return 4;
        case 0x4B: // LD C, E
            cpu->C = cpu->E;
            return 4;
        case 0x4C: // LD C, H
            cpu->C = cpu->H;
            return 4;
        case 0x4D: // LD C, L
            cpu->C = cpu->L;
            return 4;
        case 0x4E: // LD C, (HL)
            cpu->C = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x4F: // LD C, A
            cpu->C = cpu->A;
            return 4;
        case 0x50: // LD D, B
            cpu->D = cpu->B;
            return 4;
        case 0x51: // LD D, C
            cpu->D = cpu->C;
            return 4;
        case 0x52: // LD D, D
            return 4;
        case 0x53: // LD D, E
            cpu->D = cpu->E;
            return 4;
        case 0x54: // LD D, H
            cpu->D = cpu->H;
            return 4;
        case 0x55: // LD D, L
            cpu->D = cpu->L;
            return 4;
        case 0x56: // LD D, (HL)
            cpu->D = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x57: // LD D, A
            cpu->D = cpu->A;
            return 4;
        case 0x58: // LD E, B
            cpu->E = cpu->B;
            return 4;
        case 0x59: // LD E, C
            cpu->E = cpu->C;
            return 4;
        case 0x5A: // LD E, D
            cpu->E = cpu->D;
            return 4;
        case 0x5B: // LD E, E
            return 4;
        case 0x5C: // LD E, H
            cpu->E = cpu->H;
            return 4;
        case 0x5D: // LD E, L
            cpu->E = cpu->L;
            return 4;
        case 0x5E: // LD E, (HL)
            cpu->E = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x5F: // LD E, A
            cpu->E = cpu->A;
            return 4;
        case 0x60: // LD H, B
            cpu->H = cpu->B;
            return 4;
        case 0x61: // LD H, C
            cpu->H = cpu->C;
            return 4;
        case 0x62: // LD H, D
            cpu->H = cpu->D;
            return 4;
        case 0x63: // LD H, E
            cpu->H = cpu->E;
            return 4;
        case 0x64: // LD H, H
            return 4;
        case 0x65: // LD H, L
            cpu->H = cpu->L;
            return 4;
        case 0x66: // LD H, (HL)
            cpu->H = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x67: // LD H, A
            cpu->H = cpu->A;
            return 4;
        case 0x68: // LD L, B
            cpu->L = cpu->B;
            return 4;
        case 0x69: // LD L, C
            cpu->L = cpu->C;
            return 4;
        case 0x6A: // LD L, D
            cpu->L = cpu->D;
            return 4;
        case 0x6B: // LD L, E
            cpu->L = cpu->E;
            return 4;
        case 0x6C: // LD L, H
            cpu->L = cpu->H;
            return 4;
        case 0x6D: // LD L, L
            return 4;
        case 0x6E: // LD L, (HL)
            cpu->L = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x6F: // LD L, A
            cpu->L = cpu->A;
            return 4;
        case 0x70: // LD (HL), B
            cpu->memory->WriteByte(cpu->HL, cpu->B);
            return 7;
        case 0x71: // LD (HL), C
            cpu->memory->WriteByte(cpu->HL, cpu->C);
            return 7;
        case 0x72: // LD (HL), D
            cpu->memory->WriteByte(cpu->HL, cpu->D);
            return 7;
        case 0x73: // LD (HL), E
            cpu->memory->WriteByte(cpu->HL, cpu->E);
            return 7;
        case 0x74: // LD (HL), H
            cpu->memory->WriteByte(cpu->HL, cpu->H);
            return 7;
        case 0x75: // LD (HL), L
            cpu->memory->WriteByte(cpu->HL, cpu->L);
            return 7;
        case 0x76: // HALT
            cpu->HALT = true;
            cpu->PC--;
            return 4;
        case 0x77: // LD (HL), A
            cpu->memory->WriteByte(cpu->HL, cpu->A);
            return 7;
        case 0x78: // LD A, B
            cpu->A = cpu->B;
            return 4;
        case 0x79: // LD A, C
            cpu->A = cpu->C;
            return 4;
        case 0x7A: // LD A, D
            cpu->A = cpu->D;
            return 4;
        case 0x7B: // LD A, E
            cpu->A = cpu->E;
            return 4;
        case 0x7C: // LD A, H
            cpu->A = cpu->H;
            return 4;
        case 0x7D: // LD A, L
            cpu->A = cpu->L;
            return 4;
        case 0x7E: // LD A, (HL)
            cpu->A = cpu->memory->ReadByte(cpu->HL);
            return 7;
        case 0x7F: // LD A, A
            return 4;

        // Arithmetic and logic group
        case 0x80: // ADD A, B
            cpu->add8(cpu->B);
            return 4;
        case 0x81: // ADD A, C
            cpu->add8(cpu->C);
            return 4;
        case 0x82: // ADD A, D
            cpu->add8(cpu->D);
            return 4;
        case 0x83: // ADD A, E
            cpu->add8(cpu->E);
            return 4;
        case 0x84: // ADD A, H
            cpu->add8(cpu->H);
            return 4;
        case 0x85: // ADD A, L
            cpu->add8(cpu->L);
            return 4;
        case 0x86: // ADD A, (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->add8(value);
            }
            return 7;
        case 0x87: // ADD A, A
            cpu->add8(cpu->A);
            return 4;
        case 0x88: // ADC A, B
            cpu->adc8(cpu->B);
            return 4;
        case 0x89: // ADC A, C
            cpu->adc8(cpu->C);
            return 4;
        case 0x8A: // ADC A, D
            cpu->adc8(cpu->D);
            return 4;
        case 0x8B: // ADC A, E
            cpu->adc8(cpu->E);
            return 4;
        case 0x8C: // ADC A, H
            cpu->adc8(cpu->H);
            return 4;
        case 0x8D: // ADC A, L
            cpu->adc8(cpu->L);
            return 4;
        case 0x8E: // ADC A, (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->adc8(value);
            }
            return 7;
        case 0x8F: // ADC A, A
            cpu->adc8(cpu->A);
            return 4;
        case 0x90: // SUB B
            cpu->sub8(cpu->B);
            return 4;
        case 0x91: // SUB C
            cpu->sub8(cpu->C);
            return 4;
        case 0x92: // SUB D
            cpu->sub8(cpu->D);
            return 4;
        case 0x93: // SUB E
            cpu->sub8(cpu->E);
            return 4;
        case 0x94: // SUB H
            cpu->sub8(cpu->H);
            return 4;
        case 0x95: // SUB L
            cpu->sub8(cpu->L);
            return 4;
        case 0x96: // SUB (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->sub8(value);
            }
            return 7;
        case 0x97: // SUB A
            cpu->sub8(cpu->A);
            return 4;
        case 0x98: // SBC A, B
            cpu->sbc8(cpu->B);
            return 4;
        case 0x99: // SBC A, C
            cpu->sbc8(cpu->C);
            return 4;
        case 0x9A: // SBC A, D
            cpu->sbc8(cpu->D);
            return 4;
        case 0x9B: // SBC A, E
            cpu->sbc8(cpu->E);
            return 4;
        case 0x9C: // SBC A, H
            cpu->sbc8(cpu->H);
            return 4;
        case 0x9D: // SBC A, L
            cpu->sbc8(cpu->L);
            return 4;
        case 0x9E: // SBC A, (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->sbc8(value);
            }
            return 7;
        case 0x9F: // SBC A, A
            cpu->sbc8(cpu->A);
            return 4;
        case 0xA0: // AND B
            cpu->and8(cpu->B);
            return 4;
        case 0xA1: // AND C
            cpu->and8(cpu->C);
            return 4;
        case 0xA2: // AND D
            cpu->and8(cpu->D);
            return 4;
        case 0xA3: // AND E
            cpu->and8(cpu->E);
            return 4;
        case 0xA4: // AND H
            cpu->and8(cpu->H);
            return 4;
        case 0xA5: // AND L
            cpu->and8(cpu->L);
            return 4;
        case 0xA6: // AND (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->and8(value);
            }
            return 7;
        case 0xA7: // AND A
            cpu->and8(cpu->A);
            return 4;
        case 0xA8: // XOR B
            cpu->xor8(cpu->B);
            return 4;
        case 0xA9: // XOR C
            cpu->xor8(cpu->C);
            return 4;
        case 0xAA: // XOR D
            cpu->xor8(cpu->D);
            return 4;
        case 0xAB: // XOR E
            cpu->xor8(cpu->E);
            return 4;
        case 0xAC: // XOR H
            cpu->xor8(cpu->H);
            return 4;
        case 0xAD: // XOR L
            cpu->xor8(cpu->L);
            return 4;
        case 0xAE: // XOR (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->xor8(value);
            }
            return 7;
        case 0xAF: // XOR A
            cpu->xor8(cpu->A);
            return 4;
        case 0xB0: // OR B
            cpu->or8(cpu->B);
            return 4;
        case 0xB1: // OR C
            cpu->or8(cpu->C);
            return 4;
        case 0xB2: // OR D
            cpu->or8(cpu->D);
            return 4;
        case 0xB3: // OR E
            cpu->or8(cpu->E);
            return 4;
        case 0xB4: // OR H
            cpu->or8(cpu->H);
            return 4;
        case 0xB5: // OR L
            cpu->or8(cpu->L);
            return 4;
        case 0xB6: // OR (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->or8(value);
            }
            return 7;
        case 0xB7: // OR A
            cpu->or8(cpu->A);
            return 4;
        case 0xB8: // CP B
            cpu->cp8(cpu->B);
            return 4;
        case 0xB9: // CP C
            cpu->cp8(cpu->C);
            return 4;
        case 0xBA: // CP D
            cpu->cp8(cpu->D);
            return 4;
        case 0xBB: // CP E
            cpu->cp8(cpu->E);
            return 4;
        case 0xBC: // CP H
            cpu->cp8(cpu->H);
            return 4;
        case 0xBD: // CP L
            cpu->cp8(cpu->L);
            return 4;
        case 0xBE: // CP (HL)
            {
                uint8_t value = cpu->memory->ReadByte(cpu->HL);
                cpu->cp8(value);
            }
            return 7;
        case 0xBF: // CP A
            cpu->cp8(cpu->A);
            return 4;

        // RET cc instructions
        case 0xC0: // RET NZ
            if (!cpu->GetFlag(FLAG_Z)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xC1: // POP BC
            cpu->BC = cpu->Pop();
            return 10;
        case 0xC2: // JP NZ, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_Z)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xC3: // JP nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->PC = addr;
                cpu->MEMPTR = addr;
            }
            return 10;
        case 0xC4: // CALL NZ, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_Z)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xC5: // PUSH BC
            cpu->Push(cpu->BC);
            return 11;
        case 0xC6: // ADD A, n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->add8(value);
            }
            return 7;
        case 0xC7: // RST 00H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0000;
            cpu->MEMPTR = 0x0000;
            return 11;
        case 0xC8: // RET Z
            if (cpu->GetFlag(FLAG_Z)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xC9: // RET
            cpu->PC = cpu->Pop();
            cpu->MEMPTR = cpu->PC;
            return 10;
        case 0xCA: // JP Z, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_Z)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xCB: // PREFIX CB
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xCC: // CALL Z, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_Z)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xCD: // CALL nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->Push(cpu->PC);
                cpu->PC = addr;
                cpu->MEMPTR = addr;
            }
            return 17;
        case 0xCE: // ADC A, n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->adc8(value);
            }
            return 7;
        case 0xCF: // RST 08H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0008;
            cpu->MEMPTR = 0x0008;
            return 11;
        case 0xD0: // RET NC
            if (!cpu->GetFlag(FLAG_C)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xD1: // POP DE
            cpu->DE = cpu->Pop();
            return 10;
        case 0xD2: // JP NC, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_C)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xD3: // OUT (n), A
            {
                uint8_t n = cpu->ReadImmediateByte();
                uint16_t port = uint16_t(n) | (uint16_t(cpu->A) << 8);
                cpu->port->Write(port, cpu->A);
                cpu->MEMPTR = (uint16_t(cpu->A) << 8) | uint16_t((n + 1) & 0xFF);
            }
            return 11;
        case 0xD4: // CALL NC, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_C)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xD5: // PUSH DE
            cpu->Push(cpu->DE);
            return 11;
        case 0xD6: // SUB n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->sub8(value);
            }
            return 7;
        case 0xD7: // RST 10H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0010;
            cpu->MEMPTR = 0x0010;
            return 11;
        case 0xD8: // RET C
            if (cpu->GetFlag(FLAG_C)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xD9: // EXX
            {
                uint16_t tempBC = cpu->BC;
                uint16_t tempDE = cpu->DE;
                uint16_t tempHL = cpu->HL;
                cpu->BC = cpu->BC_;
                cpu->DE = cpu->DE_;
                cpu->HL = cpu->HL_;
                cpu->BC_ = tempBC;
                cpu->DE_ = tempDE;
                cpu->HL_ = tempHL;
            }
            return 4;
        case 0xDA: // JP C, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_C)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xDB: // IN A, (n)
            {
                uint8_t n = cpu->ReadImmediateByte();
                uint16_t port = uint16_t(n) | (uint16_t(cpu->A) << 8);
                cpu->A = cpu->port->Read(port);
                cpu->MEMPTR = (uint16_t(cpu->A) << 8) | uint16_t((n + 1) & 0xFF);
            }
            return 11;
        case 0xDC: // CALL C, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_C)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xDD: // PREFIX DD
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xDE: // SBC A, n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->sbc8(value);
            }
            return 7;
        case 0xDF: // RST 18H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0018;
            cpu->MEMPTR = 0x0018;
            return 11;
        case 0xE0: // RET PO
            if (!cpu->GetFlag(FLAG_PV)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xE1: // POP HL
            cpu->HL = cpu->Pop();
            return 10;
        case 0xE2: // JP PO, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_PV)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xE3: // EX (SP), HL
            {
                uint16_t temp = cpu->memory->ReadWord(cpu->SP);
                cpu->memory->WriteWord(cpu->SP, cpu->HL);
                cpu->HL = temp;
                cpu->MEMPTR = temp;
            }
            return 19;
        case 0xE4: // CALL PO, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_PV)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xE5: // PUSH HL
            cpu->Push(cpu->HL);
            return 11;
        case 0xE6: // AND n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->and8(value);
            }
            return 7;
        case 0xE7: // RST 20H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0020;
            cpu->MEMPTR = 0x0020;
            return 11;
        case 0xE8: // RET PE
            if (cpu->GetFlag(FLAG_PV)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xE9: // JP (HL)
            cpu->PC = cpu->HL;
            return 4;
        case 0xEA: // JP PE, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_PV)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xEB: // EX DE, HL
            {
                uint16_t temp = cpu->DE;
                cpu->DE = cpu->HL;
                cpu->HL = temp;
            }
            return 4;
        case 0xEC: // CALL PE, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_PV)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xED: // PREFIX ED
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xEE: // XOR n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->xor8(value);
            }
            return 7;
        case 0xEF: // RST 28H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0028;
            cpu->MEMPTR = 0x0028;
            return 11;
        case 0xF0: // RET P
            if (!cpu->GetFlag(FLAG_S)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xF1: // POP AF
            cpu->AF = cpu->Pop();
            return 10;
        case 0xF2: // JP P, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_S)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xF3: // DI
            cpu->IFF1 = false;
            cpu->IFF2 = false;
            return 4;
        case 0xF4: // CALL P, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (!cpu->GetFlag(FLAG_S)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xF5: // PUSH AF
            cpu->Push(cpu->AF);
            return 11;
        case 0xF6: // OR n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->or8(value);
            }
            return 7;
        case 0xF7: // RST 30H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0030;
            cpu->MEMPTR = 0x0030;
            return 11;
        case 0xF8: // RET M
            if (cpu->GetFlag(FLAG_S)) {
                cpu->PC = cpu->Pop();
                cpu->MEMPTR = cpu->PC;
                return 11;
            }
            return 5;
        case 0xF9: // LD SP, HL
            cpu->SP = cpu->HL;
            return 6;
        case 0xFA: // JP M, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_S)) {
                    cpu->PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xFB: // EI
            cpu->IFF1 = true;
            cpu->IFF2 = true;
            return 4;
        case 0xFC: // CALL M, nn
            {
                uint16_t addr = cpu->ReadImmediateWord();
                cpu->MEMPTR = addr;
                if (cpu->GetFlag(FLAG_S)) {
                    cpu->Push(cpu->PC);
                    cpu->PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xFD: // PREFIX FD
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xFE: // CP n
            {
                uint8_t value = cpu->ReadImmediateByte();
                cpu->cp8(value);
            }
            return 7;
        case 0xFF: // RST 38H
            cpu->Push(cpu->PC);
            cpu->PC = 0x0038;
            cpu->MEMPTR = 0x0038;
            return 11;
        default:
            // This should never happen in a correct implementation
            return 4;
    }
    
    // Default return (should not reach here)
    return 4;
}
