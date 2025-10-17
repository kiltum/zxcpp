#include "z80.hpp"
#include "memory.hpp"

// Implementation of FDCB prefixed Z80 opcodes (IY + bit manipulation instructions)
int Z80::ExecuteFDCBOpcode() {
    int8_t displacement = ReadDisplacement();
    uint8_t opcode = ReadOpcode();
    R--; // Decrement R because ReadOpcode() increments it
    uint16_t addr = uint16_t(int32_t(IY) + int32_t(displacement));
    uint8_t value = memory->memory[addr];
    MEMPTR = addr;

    // Handle rotate and shift instructions (0x00-0x3F)
    if (opcode <= 0x3F) {
        return executeRotateShiftIndexedIY(opcode, addr, value);
    }

    // Handle bit test instructions (0x40-0x7F)
    if (opcode >= 0x40 && opcode <= 0x7F) {
        uint8_t bitNum = uint8_t((opcode >> 3) & 0x07);
        bitMem(bitNum, value, uint8_t(addr >> 8));
        return 20;
    }

    // Handle reset bit instructions (0x80-0xBF)
    if (opcode >= 0x80 && opcode <= 0xBF) {
        return executeResetBitIndexedIY(opcode, addr, value);
    }

    // Handle set bit instructions (0xC0-0xFF)
    if (opcode >= 0xC0) {
        return executeSetBitIndexedIY(opcode, addr, value);
    }

    // Unimplemented opcode
    return 23;
}

// executeRotateShiftIndexedIY handles rotate and shift instructions for IY indexed addressing
int Z80::executeRotateShiftIndexedIY(uint8_t opcode, uint16_t addr, uint8_t value) {
    // Determine operation type from opcode bits 3-5
    uint8_t opType = (opcode >> 3) & 0x07;
    // Determine register from opcode bits 0-2
    uint8_t reg = opcode & 0x07;

    // Perform the operation
    uint8_t result;
    switch (opType) {
    case 0: // RLC
        result = rlc(value);
        break;
    case 1: // RRC
        result = rrc(value);
        break;
    case 2: // RL
        result = rl(value);
        break;
    case 3: // RR
        result = rr(value);
        break;
    case 4: // SLA
        result = sla(value);
        break;
    case 5: // SRA
        result = sra(value);
        break;
    case 6: // SLL (Undocumented)
        result = sll(value);
        break;
    case 7: // SRL
        result = srl(value);
        break;
    default:
        result = value;
        break;
    }

    // Store result in memory
    memory->memory[addr] = result;

    // Store result in register if needed (except for (HL) case)
    if (reg != 6) { // reg 6 is (HL) - no register store needed
        switch (reg) {
        case 0:
            B = result;
            break;
        case 1:
            C = result;
            break;
        case 2:
            D = result;
            break;
        case 3:
            E = result;
            break;
        case 4:
            H = result;
            break;
        case 5:
            L = result;
            break;
        case 7:
            A = result;
            break;
        }
    }

    return 23;
}

// executeResetBitIndexedIY handles reset bit instructions for IY indexed addressing
int Z80::executeResetBitIndexedIY(uint8_t opcode, uint16_t addr, uint8_t value) {
    uint8_t bitNum = uint8_t((opcode >> 3) & 0x07);
    uint8_t reg = opcode & 0x07;

    uint8_t result = res(bitNum, value);
    memory->memory[addr] = result;

    // Store result in register if needed (except for (HL) case)
    if (reg != 6) { // reg 6 is (HL) - no register store needed
        switch (reg) {
        case 0:
            B = result;
            break;
        case 1:
            C = result;
            break;
        case 2:
            D = result;
            break;
        case 3:
            E = result;
            break;
        case 4:
            H = result;
            break;
        case 5:
            L = result;
            break;
        case 7:
            A = result;
            break;
        }
    }

    return 23;
}

// executeSetBitIndexedIY handles set bit instructions for IY indexed addressing
int Z80::executeSetBitIndexedIY(uint8_t opcode, uint16_t addr, uint8_t value) {
    uint8_t bitNum = uint8_t((opcode >> 3) & 0x07);
    uint8_t reg = opcode & 0x07;

    uint8_t result = set(bitNum, value);
    memory->memory[addr] = result;

    // Store result in register if needed (except for (HL) case)
    if (reg != 6) { // reg 6 is (HL) - no register store needed
        switch (reg) {
        case 0:
            B = result;
            break;
        case 1:
            C = result;
            break;
        case 2:
            D = result;
            break;
        case 3:
            E = result;
            break;
        case 4:
            H = result;
            break;
        case 5:
            L = result;
            break;
        case 7:
            A = result;
            break;
        }
    }

    return 23;
}
