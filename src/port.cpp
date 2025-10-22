#include "port.hpp"
#include <stdexcept>

Port::Port() {
    // Constructor implementation
}

void Port::RegisterWriteHandler(uint16_t port, WriteHandler handler) {
    // Add the handler to the vector for this port
    writeHandlers[port].push_back(handler);
}

void Port::RegisterReadHandler(uint16_t port, ReadHandler handler) {
    // Check if a read handler is already registered for this port
    if (readHandlers.find(port) != readHandlers.end()) {
        // Optionally throw an exception or overwrite - here we'll overwrite
    }
    
    // Register the handler for this port
    readHandlers[port] = handler;
}

void Port::Write(uint16_t port, uint8_t value) {
    //printf("PORT %x\n",port);
    // // Find all handlers registered for this port
    // auto it = writeHandlers.find(port);
    // if (it != writeHandlers.end()) {
    //     // Call all registered handlers for this port
    //     for (const auto& handler : it->second) {
    //         handler(port, value);
    //         //printf("HAND\n");
    //     }
    //     return; // Handler found and called
    // }
    
    // For ZX Spectrum compatibility, try masked port addressing
    // For port 0xFE, also check addresses like 0xFFFE, 0xFDFE, etc.
    uint8_t lowByte = port & 0xFF;
    auto maskedIt = writeHandlers.find(lowByte);
    if (maskedIt != writeHandlers.end()) {
        // Call all registered handlers for the masked port
        for (const auto& handler : maskedIt->second) {
            handler(port, value);
            //("Handled write %x\n",port);
            return;
        }
    }
    // printf("Handler for writing %x to %x port not found\n",value, port);
}

uint8_t Port::Read(uint16_t port) {
    // Find the handler registered for this port
    // auto it = readHandlers.find(port);
    // if (it != readHandlers.end()) {
    //     // Call the registered handler and return its result
    //     return it->second(port);
    // }
    
    // For ZX Spectrum compatibility, try masked port addressing
    uint8_t lowByte = port & 0xFF;
    auto maskedIt = readHandlers.find(lowByte);
    if (maskedIt != readHandlers.end()) {
        // Call the registered handler for the masked port and return its result
        return maskedIt->second(port);
    }
    
    // printf("Handler for reading %x port not found\n", port);
    
    // Return 0 if no handler is registered for this port
    //return uint8_t(port >> 8); // this is for FUSE test
    return 0xff;
}
