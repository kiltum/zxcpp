#ifndef PORT_HPP
#define PORT_HPP

#include <cstdint>
#include <vector>
#include <functional>
#include <unordered_map>

class Port {
public:
    // Type definitions for port handlers
    using WriteHandler = std::function<void(uint16_t port, uint8_t value)>;
    using ReadHandler = std::function<uint8_t(uint16_t port)>;

private:
    // Storage for write handlers (multiple per port)
    std::unordered_map<uint16_t, std::vector<WriteHandler>> writeHandlers;
    
    // Storage for read handlers (one per port)
    std::unordered_map<uint16_t, ReadHandler> readHandlers;

public:
    // Constructor
    Port();
    
    // Register a handler for writing to a port
    void RegisterWriteHandler(uint16_t port, WriteHandler handler);
    
    // Register a handler for reading from a port
    void RegisterReadHandler(uint16_t port, ReadHandler handler);
    
    // Write to a port (notify all registered handlers)
    void Write(uint16_t port, uint8_t value);
    
    // Read from a port (call the registered handler)
    uint8_t Read(uint16_t port);
};

#endif // PORT_HPP
