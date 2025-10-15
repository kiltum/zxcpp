#include "port.hpp"
#include <iostream>
#include <cassert>

int main() {
    Port port;
    
    // Test variables to capture handler calls
    bool writeHandler1Called = false;
    bool writeHandler2Called = false;
    bool readHandlerCalled = false;
    uint8_t readValue = 0;
    uint8_t writtenValue = 0;
    
    // Register write handlers for port 0xFD
    port.RegisterWriteHandler(0xFD, [&writeHandler1Called, &writtenValue](uint16_t port, uint8_t value) {
        writeHandler1Called = true;
        writtenValue = value;
        std::cout << "Write Handler 1 called for port 0x" << std::hex << port 
                  << " with value 0x" << std::hex << static_cast<int>(value) << std::endl;
    });
    
    port.RegisterWriteHandler(0xFD, [&writeHandler2Called, &writtenValue](uint16_t port, uint8_t value) {
        writeHandler2Called = true;
        writtenValue = value;
        std::cout << "Write Handler 2 called for port 0x" << std::hex << port 
                  << " with value 0x" << std::hex << static_cast<int>(value) << std::endl;
    });
    
    // Register read handler for port 0xFE
    port.RegisterReadHandler(0xFE, [&readHandlerCalled](uint16_t port) -> uint8_t {
        readHandlerCalled = true;
        std::cout << "Read Handler called for port 0x" << std::hex << port << std::endl;
        return 0xAB; // Return a test value
    });
    
    // Test write operation with multiple handlers
    std::cout << "Testing write operation..." << std::endl;
    port.Write(0xFD, 0x55);
    
    // Verify both write handlers were called
    assert(writeHandler1Called);
    assert(writeHandler2Called);
    assert(writtenValue == 0x55);
    
    // Test read operation
    std::cout << "Testing read operation..." << std::endl;
    uint8_t value = port.Read(0xFE);
    
    // Verify read handler was called and returned correct value
    assert(readHandlerCalled);
    assert(value == 0xAB);
    
    // Test read operation on unregistered port (should return 0)
    uint8_t unregisteredValue = port.Read(0xFF);
    assert(unregisteredValue == 0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
