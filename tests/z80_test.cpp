#include "z80.hpp"
#include "memory.hpp"
#include <iostream>

int main() {
    // Create a memory instance
    Memory mem;
    
    // Create a port instance
    Port port;
    
    // Create a Z80 instance with the memory and port pointers
    Z80 cpu(&mem, &port);
    
    // Test that we can access the memory through the Z80 class
    // (This would normally be done within Z80 methods)
    mem.WriteByte(0x1000, 0xAB);
    uint8_t value = mem.ReadByte(0x1000);
    
    std::cout << "Memory test: Wrote 0xAB, read 0x" << std::hex << static_cast<int>(value) << std::endl;
    
    std::cout << "Z80 class initialized successfully with memory pointer!" << std::endl;
    return 0;
}
