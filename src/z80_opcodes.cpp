#include "z80.hpp"
#include "memory.hpp"
#include "port.hpp"

// Implementation of common Z80 opcodes
int Z80::ExecuteOpcode() {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = ReadOpcode();
    
    switch (opcode) {
        // 8-bit load group
        case 0x00: // NOP
            return 4;
        case 0x01: // LD BC, nn
            BC = ReadImmediateWord();
            return 10;
        case 0x02: // LD (BC), A
            memory->memory[BC] = A;
            MEMPTR = (uint16_t(A) << 8) | (uint16_t(BC + 1) & 0xff);
            return 7;
        case 0x03: // INC BC
            BC++;
            return 6;
        case 0x04: // INC B
            B = inc8(B);
            return 4;
        case 0x05: // DEC B
            B = dec8(B);
            return 4;
        case 0x06: // LD B, n
            B = ReadImmediateByte();
            return 7;
        case 0x07: // RLCA
            rlca();
            return 4;
        case 0x08: // EX AF, AF'
            {
                uint16_t temp = AF;
                AF = AF_;
                AF_ = temp;
            }
            return 4;
        case 0x09: // ADD HL, BC
            {
                uint16_t result = add16(HL, BC);
                MEMPTR = HL + 1;
                HL = result;
            }
            return 11;
        case 0x0A: // LD A, (BC)
            A = memory->memory[BC];
            MEMPTR = BC + 1;
            return 7;
        case 0x0B: // DEC BC
            BC--;
            return 6;
        case 0x0C: // INC C
            C = inc8(C);
            return 4;
        case 0x0D: // DEC C
            C = dec8(C);
            return 4;
        case 0x0E: // LD C, n
            C = ReadImmediateByte();
            return 7;
        case 0x0F: // RRCA
            rrca();
            return 4;
        case 0x10: // DJNZ e
            B--;
            if (B != 0) {
                int8_t offset = ReadDisplacement();
                MEMPTR = PC + uint16_t(int32_t(offset));
                PC = uint16_t(int32_t(PC) + int32_t(offset));
                return 13;
            }
            PC++; // Skip the offset byte
            return 8;
        case 0x11: // LD DE, nn
            DE = ReadImmediateWord();
            return 10;
        case 0x12: // LD (DE), A
            memory->memory[DE] = A;
            MEMPTR = (uint16_t(A) << 8) | (uint16_t(DE + 1) & 0xff);
            return 7;
        case 0x13: // INC DE
            DE++;
            return 6;
        case 0x14: // INC D
            D = inc8(D);
            return 4;
        case 0x15: // DEC D
            D = dec8(D);
            return 4;
        case 0x16: // LD D, n
            D = ReadImmediateByte();
            return 7;
        case 0x17: // RLA
            rla();
            return 4;
        case 0x18: // JR e
            {
                int8_t offset = ReadDisplacement();
                MEMPTR = PC + uint16_t(int32_t(offset));
                PC = uint16_t(int32_t(PC) + int32_t(offset));
            }
            return 12;
        case 0x19: // ADD HL, DE
            {
                uint16_t result = add16(HL, DE);
                MEMPTR = HL + 1;
                HL = result;
            }
            return 11;
        case 0x1A: // LD A, (DE)
            A = memory->memory[DE];
            MEMPTR = DE + 1;
            return 7;
        case 0x1B: // DEC DE
            DE--;
            return 6;
        case 0x1C: // INC E
            E = inc8(E);
            return 4;
        case 0x1D: // DEC E
            E = dec8(E);
            return 4;
        case 0x1E: // LD E, n
            E = ReadImmediateByte();
            return 7;
        case 0x1F: // RRA
            rra();
            return 4;
        case 0x20: // JR NZ, e
            if (!GetFlag(FLAG_Z)) {
                int8_t offset = ReadDisplacement();
                MEMPTR = PC + uint16_t(int32_t(offset));
                PC = uint16_t(int32_t(PC) + int32_t(offset));
                return 12;
            }
            PC++; // Skip the offset byte
            return 7;
        case 0x21: // LD HL, nn
            HL = ReadImmediateWord();
            return 10;
        case 0x22: // LD (nn), HL
            {
                uint16_t addr = ReadImmediateWord();
                memory->WriteWord(addr, HL);
                MEMPTR = addr + 1;
            }
            return 16;
        case 0x23: // INC HL
            HL++;
            return 6;
        case 0x24: // INC H
            H = inc8(H);
            return 4;
        case 0x25: // DEC H
            H = dec8(H);
            return 4;
        case 0x26: // LD H, n
            H = ReadImmediateByte();
            return 7;
        case 0x27: // DAA
            daa();
            return 4;
        case 0x28: // JR Z, e
            if (GetFlag(FLAG_Z)) {
                int8_t offset = ReadDisplacement();
                MEMPTR = uint16_t(int32_t(PC) + int32_t(offset));
                PC = uint16_t(int32_t(PC) + int32_t(offset));
                return 12;
            }
            ReadDisplacement(); // Skip the offset byte
            return 7;
        case 0x29: // ADD HL, HL
            {
                uint16_t result = add16(HL, HL);
                MEMPTR = HL + 1;
                HL = result;
            }
            return 11;
        case 0x2A: // LD HL, (nn)
            {
                uint16_t addr = ReadImmediateWord();
                HL = memory->ReadWord(addr);
                MEMPTR = addr + 1;
            }
            return 16;
        case 0x2B: // DEC HL
            HL--;
            return 6;
        case 0x2C: // INC L
            L = inc8(L);
            return 4;
        case 0x2D: // DEC L
            L = dec8(L);
            return 4;
        case 0x2E: // LD L, n
            L = ReadImmediateByte();
            return 7;
        case 0x2F: // CPL
            cpl();
            return 4;
        case 0x30: // JR NC, e
            if (!GetFlag(FLAG_C)) {
                int8_t offset = ReadDisplacement();
                MEMPTR = PC + uint16_t(int32_t(offset));
                PC = uint16_t(int32_t(PC) + int32_t(offset));
                return 12;
            }
            PC++; // Skip the offset byte
            return 7;
        case 0x31: // LD SP, nn
            SP = ReadImmediateWord();
            return 10;
        case 0x32: // LD (nn), A
            {
                uint16_t addr = ReadImmediateWord();
                memory->memory[addr] = A;
                MEMPTR = (uint16_t(A) << 8) | ((addr + 1) & 0xFF);
            }
            return 13;
        case 0x33: // INC SP
            SP++;
            return 6;
        case 0x34: // INC (HL)
            {
                uint8_t value = memory->memory[HL];
                uint8_t result = inc8(value);
                memory->memory[HL] = result;
            }
            return 11;
        case 0x35: // DEC (HL)
            {
                uint8_t value = memory->memory[HL];
                uint8_t result = dec8(value);
                memory->memory[HL] = result;
            }
            return 11;
        case 0x36: // LD (HL), n
            {
                uint8_t value = ReadImmediateByte();
                memory->memory[HL] = value;
            }
            return 10;
        case 0x37: // SCF
            scf();
            return 4;
        case 0x38: // JR C, e
            if (GetFlag(FLAG_C)) {
                int8_t offset = ReadDisplacement();
                MEMPTR = PC + uint16_t(int32_t(offset));
                PC = uint16_t(int32_t(PC) + int32_t(offset));
                return 12;
            }
            PC++; // Skip the offset byte
            return 7;
        case 0x39: // ADD HL, SP
            {
                uint16_t result = add16(HL, SP);
                MEMPTR = HL + 1;
                HL = result;
            }
            return 11;
        case 0x3A: // LD A, (nn)
            {
                uint16_t addr = ReadImmediateWord();
                A = memory->memory[addr];
                MEMPTR = addr + 1;
            }
            return 13;
        case 0x3B: // DEC SP
            SP--;
            return 6;
        case 0x3C: // INC A
            A = inc8(A);
            return 4;
        case 0x3D: // DEC A
            A = dec8(A);
            return 4;
        case 0x3E: // LD A, n
            A = ReadImmediateByte();
            return 7;
        case 0x3F: // CCF
            ccf();
            return 4;

        // LD r, r' instructions
        case 0x40: // LD B, B
            return 4;
        case 0x41: // LD B, C
            B = C;
            return 4;
        case 0x42: // LD B, D
            B = D;
            return 4;
        case 0x43: // LD B, E
            B = E;
            return 4;
        case 0x44: // LD B, H
            B = H;
            return 4;
        case 0x45: // LD B, L
            B = L;
            return 4;
        case 0x46: // LD B, (HL)
            B = memory->memory[HL];
            return 7;
        case 0x47: // LD B, A
            B = A;
            return 4;
        case 0x48: // LD C, B
            C = B;
            return 4;
        case 0x49: // LD C, C
            return 4;
        case 0x4A: // LD C, D
            C = D;
            return 4;
        case 0x4B: // LD C, E
            C = E;
            return 4;
        case 0x4C: // LD C, H
            C = H;
            return 4;
        case 0x4D: // LD C, L
            C = L;
            return 4;
        case 0x4E: // LD C, (HL)
            C = memory->memory[HL];
            return 7;
        case 0x4F: // LD C, A
            C = A;
            return 4;
        case 0x50: // LD D, B
            D = B;
            return 4;
        case 0x51: // LD D, C
            D = C;
            return 4;
        case 0x52: // LD D, D
            return 4;
        case 0x53: // LD D, E
            D = E;
            return 4;
        case 0x54: // LD D, H
            D = H;
            return 4;
        case 0x55: // LD D, L
            D = L;
            return 4;
        case 0x56: // LD D, (HL)
            D = memory->memory[HL];
            return 7;
        case 0x57: // LD D, A
            D = A;
            return 4;
        case 0x58: // LD E, B
            E = B;
            return 4;
        case 0x59: // LD E, C
            E = C;
            return 4;
        case 0x5A: // LD E, D
            E = D;
            return 4;
        case 0x5B: // LD E, E
            return 4;
        case 0x5C: // LD E, H
            E = H;
            return 4;
        case 0x5D: // LD E, L
            E = L;
            return 4;
        case 0x5E: // LD E, (HL)
            E = memory->memory[HL];
            return 7;
        case 0x5F: // LD E, A
            E = A;
            return 4;
        case 0x60: // LD H, B
            H = B;
            return 4;
        case 0x61: // LD H, C
            H = C;
            return 4;
        case 0x62: // LD H, D
            H = D;
            return 4;
        case 0x63: // LD H, E
            H = E;
            return 4;
        case 0x64: // LD H, H
            return 4;
        case 0x65: // LD H, L
            H = L;
            return 4;
        case 0x66: // LD H, (HL)
            H = memory->memory[HL];
            return 7;
        case 0x67: // LD H, A
            H = A;
            return 4;
        case 0x68: // LD L, B
            L = B;
            return 4;
        case 0x69: // LD L, C
            L = C;
            return 4;
        case 0x6A: // LD L, D
            L = D;
            return 4;
        case 0x6B: // LD L, E
            L = E;
            return 4;
        case 0x6C: // LD L, H
            L = H;
            return 4;
        case 0x6D: // LD L, L
            return 4;
        case 0x6E: // LD L, (HL)
            L = memory->memory[HL];
            return 7;
        case 0x6F: // LD L, A
            L = A;
            return 4;
        case 0x70: // LD (HL), B
            memory->memory[HL] = B;
            return 7;
        case 0x71: // LD (HL), C
            memory->memory[HL] = C;
            return 7;
        case 0x72: // LD (HL), D
            memory->memory[HL] = D;
            return 7;
        case 0x73: // LD (HL), E
            memory->memory[HL] = E;
            return 7;
        case 0x74: // LD (HL), H
            memory->memory[HL] = H;
            return 7;
        case 0x75: // LD (HL), L
            memory->memory[HL] = L;
            return 7;
        case 0x76: // HALT
            HALT = true;
            PC--;
            return 4;
        case 0x77: // LD (HL), A
            memory->memory[HL] = A;
            return 7;
        case 0x78: // LD A, B
            A = B;
            return 4;
        case 0x79: // LD A, C
            A = C;
            return 4;
        case 0x7A: // LD A, D
            A = D;
            return 4;
        case 0x7B: // LD A, E
            A = E;
            return 4;
        case 0x7C: // LD A, H
            A = H;
            return 4;
        case 0x7D: // LD A, L
            A = L;
            return 4;
        case 0x7E: // LD A, (HL)
            A = memory->memory[HL];
            return 7;
        case 0x7F: // LD A, A
            return 4;

        // Arithmetic and logic group
        case 0x80: // ADD A, B
            add8(B);
            return 4;
        case 0x81: // ADD A, C
            add8(C);
            return 4;
        case 0x82: // ADD A, D
            add8(D);
            return 4;
        case 0x83: // ADD A, E
            add8(E);
            return 4;
        case 0x84: // ADD A, H
            add8(H);
            return 4;
        case 0x85: // ADD A, L
            add8(L);
            return 4;
        case 0x86: // ADD A, (HL)
            {
                uint8_t value = memory->memory[HL];
                add8(value);
            }
            return 7;
        case 0x87: // ADD A, A
            add8(A);
            return 4;
        case 0x88: // ADC A, B
            adc8(B);
            return 4;
        case 0x89: // ADC A, C
            adc8(C);
            return 4;
        case 0x8A: // ADC A, D
            adc8(D);
            return 4;
        case 0x8B: // ADC A, E
            adc8(E);
            return 4;
        case 0x8C: // ADC A, H
            adc8(H);
            return 4;
        case 0x8D: // ADC A, L
            adc8(L);
            return 4;
        case 0x8E: // ADC A, (HL)
            {
                uint8_t value = memory->memory[HL];
                adc8(value);
            }
            return 7;
        case 0x8F: // ADC A, A
            adc8(A);
            return 4;
        case 0x90: // SUB B
            sub8(B);
            return 4;
        case 0x91: // SUB C
            sub8(C);
            return 4;
        case 0x92: // SUB D
            sub8(D);
            return 4;
        case 0x93: // SUB E
            sub8(E);
            return 4;
        case 0x94: // SUB H
            sub8(H);
            return 4;
        case 0x95: // SUB L
            sub8(L);
            return 4;
        case 0x96: // SUB (HL)
            {
                uint8_t value = memory->memory[HL];
                sub8(value);
            }
            return 7;
        case 0x97: // SUB A
            sub8(A);
            return 4;
        case 0x98: // SBC A, B
            sbc8(B);
            return 4;
        case 0x99: // SBC A, C
            sbc8(C);
            return 4;
        case 0x9A: // SBC A, D
            sbc8(D);
            return 4;
        case 0x9B: // SBC A, E
            sbc8(E);
            return 4;
        case 0x9C: // SBC A, H
            sbc8(H);
            return 4;
        case 0x9D: // SBC A, L
            sbc8(L);
            return 4;
        case 0x9E: // SBC A, (HL)
            {
                uint8_t value = memory->memory[HL];
                sbc8(value);
            }
            return 7;
        case 0x9F: // SBC A, A
            sbc8(A);
            return 4;
        case 0xA0: // AND B
            and8(B);
            return 4;
        case 0xA1: // AND C
            and8(C);
            return 4;
        case 0xA2: // AND D
            and8(D);
            return 4;
        case 0xA3: // AND E
            and8(E);
            return 4;
        case 0xA4: // AND H
            and8(H);
            return 4;
        case 0xA5: // AND L
            and8(L);
            return 4;
        case 0xA6: // AND (HL)
            {
                uint8_t value = memory->memory[HL];
                and8(value);
            }
            return 7;
        case 0xA7: // AND A
            and8(A);
            return 4;
        case 0xA8: // XOR B
            xor8(B);
            return 4;
        case 0xA9: // XOR C
            xor8(C);
            return 4;
        case 0xAA: // XOR D
            xor8(D);
            return 4;
        case 0xAB: // XOR E
            xor8(E);
            return 4;
        case 0xAC: // XOR H
            xor8(H);
            return 4;
        case 0xAD: // XOR L
            xor8(L);
            return 4;
        case 0xAE: // XOR (HL)
            {
                uint8_t value = memory->memory[HL];
                xor8(value);
            }
            return 7;
        case 0xAF: // XOR A
            xor8(A);
            return 4;
        case 0xB0: // OR B
            or8(B);
            return 4;
        case 0xB1: // OR C
            or8(C);
            return 4;
        case 0xB2: // OR D
            or8(D);
            return 4;
        case 0xB3: // OR E
            or8(E);
            return 4;
        case 0xB4: // OR H
            or8(H);
            return 4;
        case 0xB5: // OR L
            or8(L);
            return 4;
        case 0xB6: // OR (HL)
            {
                uint8_t value = memory->memory[HL];
                or8(value);
            }
            return 7;
        case 0xB7: // OR A
            or8(A);
            return 4;
        case 0xB8: // CP B
            cp8(B);
            return 4;
        case 0xB9: // CP C
            cp8(C);
            return 4;
        case 0xBA: // CP D
            cp8(D);
            return 4;
        case 0xBB: // CP E
            cp8(E);
            return 4;
        case 0xBC: // CP H
            cp8(H);
            return 4;
        case 0xBD: // CP L
            cp8(L);
            return 4;
        case 0xBE: // CP (HL)
            {
                uint8_t value = memory->memory[HL];
                cp8(value);
            }
            return 7;
        case 0xBF: // CP A
            cp8(A);
            return 4;

        // RET cc instructions
        case 0xC0: // RET NZ
            if (!GetFlag(FLAG_Z)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xC1: // POP BC
            BC = Pop();
            return 10;
        case 0xC2: // JP NZ, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_Z)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xC3: // JP nn
            {
                uint16_t addr = ReadImmediateWord();
                PC = addr;
                MEMPTR = addr;
            }
            return 10;
        case 0xC4: // CALL NZ, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_Z)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xC5: // PUSH BC
            Push(BC);
            return 11;
        case 0xC6: // ADD A, n
            {
                uint8_t value = ReadImmediateByte();
                add8(value);
            }
            return 7;
        case 0xC7: // RST 00H
            Push(PC);
            PC = 0x0000;
            MEMPTR = 0x0000;
            return 11;
        case 0xC8: // RET Z
            if (GetFlag(FLAG_Z)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xC9: // RET
            PC = Pop();
            MEMPTR = PC;
            return 10;
        case 0xCA: // JP Z, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_Z)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xCB: // PREFIX CB
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xCC: // CALL Z, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_Z)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xCD: // CALL nn
            {
                uint16_t addr = ReadImmediateWord();
                Push(PC);
                PC = addr;
                MEMPTR = addr;
            }
            return 17;
        case 0xCE: // ADC A, n
            {
                uint8_t value = ReadImmediateByte();
                adc8(value);
            }
            return 7;
        case 0xCF: // RST 08H
            Push(PC);
            PC = 0x0008;
            MEMPTR = 0x0008;
            return 11;
        case 0xD0: // RET NC
            if (!GetFlag(FLAG_C)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xD1: // POP DE
            DE = Pop();
            return 10;
        case 0xD2: // JP NC, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_C)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xD3: // OUT (n), A
            {
                uint8_t n = ReadImmediateByte();
                uint16_t portw = uint16_t(n) | (uint16_t(A) << 8);
                port->Write(portw, A);
                MEMPTR = (uint16_t(A) << 8) | uint16_t((n + 1) & 0xFF);
            }
            return 11;
        case 0xD4: // CALL NC, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_C)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xD5: // PUSH DE
            Push(DE);
            return 11;
        case 0xD6: // SUB n
            {
                uint8_t value = ReadImmediateByte();
                sub8(value);
            }
            return 7;
        case 0xD7: // RST 10H
            Push(PC);
            PC = 0x0010;
            MEMPTR = 0x0010;
            return 11;
        case 0xD8: // RET C
            if (GetFlag(FLAG_C)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xD9: // EXX
            {
                uint16_t tempBC = BC;
                uint16_t tempDE = DE;
                uint16_t tempHL = HL;
                BC = BC_;
                DE = DE_;
                HL = HL_;
                BC_ = tempBC;
                DE_ = tempDE;
                HL_ = tempHL;
            }
            return 4;
        case 0xDA: // JP C, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_C)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xDB: // IN A, (n)
            {
                uint8_t n = ReadImmediateByte();
                uint16_t portr = uint16_t(n) | (uint16_t(A) << 8);
                A = port->Read(portr);
                MEMPTR = (uint16_t(A) << 8) | uint16_t((n + 1) & 0xFF);
            }
            return 11;
        case 0xDC: // CALL C, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_C)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xDD: // PREFIX DD
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xDE: // SBC A, n
            {
                uint8_t value = ReadImmediateByte();
                sbc8(value);
            }
            return 7;
        case 0xDF: // RST 18H
            Push(PC);
            PC = 0x0018;
            MEMPTR = 0x0018;
            return 11;
        case 0xE0: // RET PO
            if (!GetFlag(FLAG_PV)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xE1: // POP HL
            HL = Pop();
            return 10;
        case 0xE2: // JP PO, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_PV)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xE3: // EX (SP), HL
            {
                uint16_t temp = memory->ReadWord(SP);
                memory->WriteWord(SP, HL);
                HL = temp;
                MEMPTR = temp;
            }
            return 19;
        case 0xE4: // CALL PO, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_PV)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xE5: // PUSH HL
            Push(HL);
            return 11;
        case 0xE6: // AND n
            {
                uint8_t value = ReadImmediateByte();
                and8(value);
            }
            return 7;
        case 0xE7: // RST 20H
            Push(PC);
            PC = 0x0020;
            MEMPTR = 0x0020;
            return 11;
        case 0xE8: // RET PE
            if (GetFlag(FLAG_PV)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xE9: // JP (HL)
            PC = HL;
            return 4;
        case 0xEA: // JP PE, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_PV)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xEB: // EX DE, HL
            {
                uint16_t temp = DE;
                DE = HL;
                HL = temp;
            }
            return 4;
        case 0xEC: // CALL PE, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_PV)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xED: // PREFIX ED
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xEE: // XOR n
            {
                uint8_t value = ReadImmediateByte();
                xor8(value);
            }
            return 7;
        case 0xEF: // RST 28H
            Push(PC);
            PC = 0x0028;
            MEMPTR = 0x0028;
            return 11;
        case 0xF0: // RET P
            if (!GetFlag(FLAG_S)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xF1: // POP AF
            AF = Pop();
            return 10;
        case 0xF2: // JP P, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_S)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xF3: // DI
            IFF1 = false;
            IFF2 = false;
            return 4;
        case 0xF4: // CALL P, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (!GetFlag(FLAG_S)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xF5: // PUSH AF
            Push(AF);
            return 11;
        case 0xF6: // OR n
            {
                uint8_t value = ReadImmediateByte();
                or8(value);
            }
            return 7;
        case 0xF7: // RST 30H
            Push(PC);
            PC = 0x0030;
            MEMPTR = 0x0030;
            return 11;
        case 0xF8: // RET M
            if (GetFlag(FLAG_S)) {
                PC = Pop();
                MEMPTR = PC;
                return 11;
            }
            return 5;
        case 0xF9: // LD SP, HL
            SP = HL;
            return 6;
        case 0xFA: // JP M, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_S)) {
                    PC = addr;
                    return 10;
                }
                return 10;
            }
        case 0xFB: // EI
            IFF1 = true;
            IFF2 = true;
            return 4;
        case 0xFC: // CALL M, nn
            {
                uint16_t addr = ReadImmediateWord();
                MEMPTR = addr;
                if (GetFlag(FLAG_S)) {
                    Push(PC);
                    PC = addr;
                    return 17;
                }
                return 10;
            }
        case 0xFD: // PREFIX FD
            // This should never be reached as it's handled in ExecuteOneInstruction
            return 0;
        case 0xFE: // CP n
            {
                uint8_t value = ReadImmediateByte();
                cp8(value);
            }
            return 7;
        case 0xFF: // RST 38H
            Push(PC);
            PC = 0x0038;
            MEMPTR = 0x0038;
            return 11;
        default:
            // This should never happen in a correct implementation
            return 4;
    }
    
    // Default return (should not reach here)
    return 4;
}
