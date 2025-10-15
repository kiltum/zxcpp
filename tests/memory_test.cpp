#include "memory.hpp"
#include <iostream>
#include <cassert>

int main() {
    Memory mem;
    
    // Test byte operations
    mem.WriteByte(0x1000, 0xAB);
    assert(mem.ReadByte(0x1000) == 0xAB);
    
    // Test word operations (little-endian)
    mem.WriteWord(0x2000, 0xCD34);
    assert(mem.ReadWord(0x2000) == 0xCD34);
    
    // Verify byte order
    assert(mem.ReadByte(0x2000) == 0x34);  // Low byte
    assert(mem.ReadByte(0x2001) == 0xCD);  // High byte
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
