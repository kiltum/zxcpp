#include "memory.hpp"

Memory::Memory() {
    // Initialize all memory to zero
    for (int i = 0; i < 65536; i++) {
        memory[i] = 0;
    }
}

void Memory::WriteByte(uint16_t address, uint8_t value) {
    memory[address] = value;
}

// uint16_t Memory::ReadWord(uint16_t address) {
//     // Read word in little-endian format (least significant byte first)
//     uint8_t low = memory[address];
//     uint8_t high = memory[address + 1];
//     return (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
// }

// void Memory::WriteWord(uint16_t address, uint16_t value) {
//     // Write word in little-endian format (least significant byte first)
//     memory[address] = static_cast<uint8_t>(value & 0xFF);
//     memory[address + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
// }

// inline void Memory::WriteWord(uint16_t address, uint16_t value) noexcept
// {
//     *reinterpret_cast<uint16_t*>(memory + address) = value;   // one store
// }