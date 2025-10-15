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
    // Find all handlers registered for this port
    auto it = writeHandlers.find(port);
    if (it != writeHandlers.end()) {
        // Call all registered handlers for this port
        for (const auto& handler : it->second) {
            handler(port, value);
        }
    }
}

uint8_t Port::Read(uint16_t port) {
    // Find the handler registered for this port
    auto it = readHandlers.find(port);
    if (it != readHandlers.end()) {
        // Call the registered handler and return its result
        return it->second(port);
    }
    
    // Return 0 if no handler is registered for this port
    return 0;
}
