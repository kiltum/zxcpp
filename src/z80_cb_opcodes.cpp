#include "z80.hpp"
#include "memory.hpp"

// Implementation of CB prefixed Z80 opcodes (bit manipulation instructions)
int Z80::ExecuteCBOpcode() {
    // Read the opcode from memory at the current program counter
    uint8_t opcode = ReadOpcode();
    
    // Handle rotate and shift instructions (0x00-0x3F)
    if (opcode <= 0x3F) {
        // Determine operation type from opcode bits 3-5
        uint8_t opType = (opcode >> 3) & 0x07;
        // Determine register from opcode bits 0-2
        uint8_t reg = opcode & 0x07;

        // Handle (HL) special case
        if (reg == 6) {
            uint16_t addr = HL;
            uint8_t value = memory->ReadByte(addr);

            switch (opType) {
            case 0: // RLC
                {
                    uint8_t result = rlc(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 1: // RRC
                {
                    uint8_t result = rrc(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 2: // RL
                {
                    uint8_t result = rl(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 3: // RR
                {
                    uint8_t result = rr(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 4: // SLA
                {
                    uint8_t result = sla(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 5: // SRA
                {
                    uint8_t result = sra(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 6: // SLL (Undocumented)
                {
                    uint8_t result = sll(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            case 7: // SRL
                {
                    uint8_t result = srl(value);
                    memory->WriteByte(addr, result);
                }
                return 15;
            }
        } else {
            // Handle regular registers
            switch (opType) {
            case 0: // RLC
                switch (reg) {
                case 0:
                    B = rlc(B);
                    break;
                case 1:
                    C = rlc(C);
                    break;
                case 2:
                    D = rlc(D);
                    break;
                case 3:
                    E = rlc(E);
                    break;
                case 4:
                    H = rlc(H);
                    break;
                case 5:
                    L = rlc(L);
                    break;
                case 7:
                    A = rlc(A);
                    break;
                }
                return 8;
            case 1: // RRC
                switch (reg) {
                case 0:
                    B = rrc(B);
                    break;
                case 1:
                    C = rrc(C);
                    break;
                case 2:
                    D = rrc(D);
                    break;
                case 3:
                    E = rrc(E);
                    break;
                case 4:
                    H = rrc(H);
                    break;
                case 5:
                    L = rrc(L);
                    break;
                case 7:
                    A = rrc(A);
                    break;
                }
                return 8;
            case 2: // RL
                switch (reg) {
                case 0:
                    B = rl(B);
                    break;
                case 1:
                    C = rl(C);
                    break;
                case 2:
                    D = rl(D);
                    break;
                case 3:
                    E = rl(E);
                    break;
                case 4:
                    H = rl(H);
                    break;
                case 5:
                    L = rl(L);
                    break;
                case 7:
                    A = rl(A);
                    break;
                }
                return 8;
            case 3: // RR
                switch (reg) {
                case 0:
                    B = rr(B);
                    break;
                case 1:
                    C = rr(C);
                    break;
                case 2:
                    D = rr(D);
                    break;
                case 3:
                    E = rr(E);
                    break;
                case 4:
                    H = rr(H);
                    break;
                case 5:
                    L = rr(L);
                    break;
                case 7:
                    A = rr(A);
                    break;
                }
                return 8;
            case 4: // SLA
                switch (reg) {
                case 0:
                    B = sla(B);
                    break;
                case 1:
                    C = sla(C);
                    break;
                case 2:
                    D = sla(D);
                    break;
                case 3:
                    E = sla(E);
                    break;
                case 4:
                    H = sla(H);
                    break;
                case 5:
                    L = sla(L);
                    break;
                case 7:
                    A = sla(A);
                    break;
                }
                return 8;
            case 5: // SRA
                switch (reg) {
                case 0:
                    B = sra(B);
                    break;
                case 1:
                    C = sra(C);
                    break;
                case 2:
                    D = sra(D);
                    break;
                case 3:
                    E = sra(E);
                    break;
                case 4:
                    H = sra(H);
                    break;
                case 5:
                    L = sra(L);
                    break;
                case 7:
                    A = sra(A);
                    break;
                }
                return 8;
            case 6: // SLL (Undocumented)
                switch (reg) {
                case 0:
                    B = sll(B);
                    break;
                case 1:
                    C = sll(C);
                    break;
                case 2:
                    D = sll(D);
                    break;
                case 3:
                    E = sll(E);
                    break;
                case 4:
                    H = sll(H);
                    break;
                case 5:
                    L = sll(L);
                    break;
                case 7:
                    A = sll(A);
                    break;
                }
                return 8;
            case 7: // SRL
                switch (reg) {
                case 0:
                    B = srl(B);
                    break;
                case 1:
                    C = srl(C);
                    break;
                case 2:
                    D = srl(D);
                    break;
                case 3:
                    E = srl(E);
                    break;
                case 4:
                    H = srl(H);
                    break;
                case 5:
                    L = srl(L);
                    break;
                case 7:
                    A = srl(A);
                    break;
                }
                return 8;
            }
        }
    }

    // Handle bit test instructions (0x40-0x7F)
    if (opcode >= 0x40 && opcode <= 0x7F) {
        uint8_t bitNum = (opcode >> 3) & 0x07;
        uint8_t reg = opcode & 0x07;

        // Handle (HL) special case
        if (reg == 6) {
            uint8_t value = memory->ReadByte(HL);
            bitMem(bitNum, value, uint8_t(MEMPTR >> 8));
            return 12;
        } else {
            // Handle regular registers
            uint8_t regValue;
            switch (reg) {
            case 0:
                regValue = B;
                break;
            case 1:
                regValue = C;
                break;
            case 2:
                regValue = D;
                break;
            case 3:
                regValue = E;
                break;
            case 4:
                regValue = H;
                break;
            case 5:
                regValue = L;
                break;
            case 7:
                regValue = A;
                break;
            }
            bit(bitNum, regValue);
            return 8;
        }
    }

    // Handle reset bit instructions (0x80-0xBF)
    if (opcode >= 0x80 && opcode <= 0xBF) {
        uint8_t bitNum = (opcode >> 3) & 0x07;
        uint8_t reg = opcode & 0x07;

        // Handle (HL) special case
        if (reg == 6) {
            uint16_t addr = HL;
            uint8_t value = memory->ReadByte(addr);
            uint8_t result = res(bitNum, value);
            memory->WriteByte(addr, result);
            return 15;
        } else {
            // Handle regular registers
            switch (reg) {
            case 0:
                B = res(bitNum, B);
                break;
            case 1:
                C = res(bitNum, C);
                break;
            case 2:
                D = res(bitNum, D);
                break;
            case 3:
                E = res(bitNum, E);
                break;
            case 4:
                H = res(bitNum, H);
                break;
            case 5:
                L = res(bitNum, L);
                break;
            case 7:
                A = res(bitNum, A);
                break;
            }
            return 8;
        }
    }

    // Handle set bit instructions (0xC0-0xFF)
    if (opcode >= 0xC0) {
        uint8_t bitNum = (opcode >> 3) & 0x07;
        uint8_t reg = opcode & 0x07;

        // Handle (HL) special case
        if (reg == 6) {
            uint16_t addr = HL;
            uint8_t value = memory->ReadByte(addr);
            uint8_t result = set(bitNum, value);
            memory->WriteByte(addr, result);
            return 15;
        } else {
            // Handle regular registers
            switch (reg) {
            case 0:
                B = set(bitNum, B);
                break;
            case 1:
                C = set(bitNum, C);
                break;
            case 2:
                D = set(bitNum, D);
                break;
            case 3:
                E = set(bitNum, E);
                break;
            case 4:
                H = set(bitNum, H);
                break;
            case 5:
                L = set(bitNum, L);
                break;
            case 7:
                A = set(bitNum, A);
                break;
            }
            return 8;
        }
    }

    // Unimplemented opcode
    return 8;
}
