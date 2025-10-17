#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

class Memory
{
public:
    uint8_t memory[65536]; // 64KB of memory

    // Constructor
    Memory();

    // Write a byte to memory
    void WriteByte(uint16_t address, uint8_t value);

    inline void WriteWord(uint16_t address, uint16_t value) noexcept
    {
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
        *reinterpret_cast<uint16_t *>(memory + address) = value;
#else
        std::memcpy(memory + address, &value, sizeof(value));
#endif
    }

    inline uint16_t ReadWord(uint16_t address) const noexcept
    {
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
        return *reinterpret_cast<const uint16_t *>(memory + address);
#else
        uint16_t v;
        std::memcpy(&v, memory + address, sizeof(v));
        return v;
#endif
    }

};

#endif // MEMORY_HPP
