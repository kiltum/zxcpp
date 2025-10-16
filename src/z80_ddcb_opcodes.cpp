#include "z80.hpp"
#include "memory.hpp"

// Implementation of DD CB prefixed Z80 opcodes (IX with displacement and CB operations)
int Z80::executeDDCBOpcode() {
    // For DD CB prefixed instructions, R should be incremented by 2 total
    // We've already incremented R once for the DD prefix and once for the CB prefix
    // So we need to adjust by -1 to get the correct total increment of 2
    uint8_t originalR = R;

    int8_t displacement = ReadDisplacement();
    uint8_t opcode = ReadOpcode();

    // Adjust R register - DD CB instructions should increment R by 2 total
    // We've already incremented twice (DD and CB), so we need to subtract 1
    // to get the correct total of 2 increments
    R = originalR;

    uint16_t addr = uint16_t(int32_t(IX) + int32_t(displacement));
    uint8_t value = memory->ReadByte(addr);

    // Handle rotate and shift instructions (0x00-0x3F)
    if (opcode <= 0x3F) {
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
        memory->WriteByte(addr, result);

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

        MEMPTR = addr;
        return 23;
    }

    // Handle bit test instructions (0x40-0x7F)
    if (opcode >= 0x40 && opcode <= 0x7F) {
        uint8_t bitNum = (opcode >> 3) & 0x07;
        bitMem(bitNum, value, uint8_t(addr >> 8));
        MEMPTR = addr;
        return 20;
    }

    // Handle reset bit instructions (0x80-0xBF)
    if (opcode >= 0x80 && opcode <= 0xBF) {
        uint8_t bitNum = (opcode >> 3) & 0x07;
        uint8_t reg = opcode & 0x07;

        uint8_t result = res(bitNum, value);
        memory->WriteByte(addr, result);

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

        MEMPTR = addr;
        return 23;
    }

    // Handle set bit instructions (0xC0-0xFF)
    if (opcode >= 0xC0) {
        uint8_t bitNum = (opcode >> 3) & 0x07;
        uint8_t reg = opcode & 0x07;

        uint8_t result = set(bitNum, value);
        memory->WriteByte(addr, result);

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

        MEMPTR = addr;
        return 23;
    }

    // Unimplemented opcode
    return 23;
}
