#include "port.hpp"

Port::Port()
{
    // Constructor implementation
}

void Port::Clear(void)
{
    writeHandlers.clear();
    readHandlers.clear();
}

void Port::RegisterWriteHandler(uint16_t port, WriteHandler handler)
{
    // Add the handler to the vector for this port
    writeHandlers[port].push_back(handler);
}

void Port::RegisterReadHandler(uint16_t port, ReadHandler handler)
{
    // Check if a read handler is already registered for this port
    if (readHandlers.find(port) != readHandlers.end())
    {
        // Optionally throw an exception or overwrite - here we'll overwrite
    }

    // Register the handler for this port
    readHandlers[port] = handler;
}

void Port::Write(uint16_t port, uint8_t value)
{
    // For ZX Spectrum compatibility, try masked port addressing
    // For port 0xFE, also check addresses like 0xFFFE, 0xFDFE, etc.
    uint8_t lowByte = port & 0xFF;
    auto maskedIt = writeHandlers.find(lowByte);
    if (maskedIt != writeHandlers.end())
    {
        // Call all registered handlers for the masked port
        for (const auto &handler : maskedIt->second)
        {
            handler(port, value);
        }
    }
    else
        printf("Handler for writing %x to %x port not found\n", value, port);
}

uint8_t Port::Read(uint16_t port)
{
    // For ZX Spectrum compatibility, try masked port addressing
    uint8_t lowByte = port & 0xFF;
    auto maskedIt = readHandlers.find(lowByte);
    if (maskedIt != readHandlers.end())
    {
        // Call the registered handler for the masked port and return its result
        return maskedIt->second(port);
    }
    printf("Handler for reading %x port not found\n", port);
    // Return 0 if no handler is registered for this port
    return uint8_t(port >> 8); // this is for FUSE test and Floating bus test
}
