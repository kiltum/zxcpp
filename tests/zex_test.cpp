#include "z80.hpp"
#include "memory.hpp"
#include "port.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

// ExtendedMemory extends Memory to provide additional functionality
class ExtendedMemory : public Memory
{
public:
    // Load data into memory
    void LoadData(uint16_t address, const uint8_t *dataPtr, size_t length)
    {
        for (size_t i = 0; i < length; i++)
        {
            if (address + i < 0x10000)
            {
                WriteByte(address + i, dataPtr[i]);
            }
        }
    }
};

// IOHandler handles I/O operations
class IOHandler : public Port
{
private:
    Z80 *cpu;
    Memory *memory;

public:
    IOHandler(Z80 *cpu, Memory *memory) : cpu(cpu), memory(memory) {}

    // handleBDOSCall handles CP/M BDOS calls
    bool HandleBDOSCall()
    {
        // BDOS is called by jumping to 0x0005
        // Function number is in register C
        // Parameters are in other registers depending on the function

        uint8_t function = cpu->C;

        switch (function)
        {
        case 2: // Print character (character in E)
            std::cout << (char)cpu->E;
            std::cout.flush();
            break;
        case 9: // Print string (string address in DE)
        {
            uint16_t addr = (cpu->D << 8) | cpu->E; // Get DE register value
            // Read characters from memory until we encounter '$'
            while (true)
            {
                uint8_t ch = memory->ReadByte(addr);
                if (ch == '$')
                {
                    break;
                }
                std::cout << (char)ch;
                addr++;
            }
            std::cout.flush();
        }
        break;
        default:
            // Unknown BDOS function
            return false;
        }

        // In a real CP/M system, after handling the BDOS call,
        // execution would return to the caller via a RET instruction.
        // We simulate this by popping the return address from the stack
        // and setting the PC to that address.

        // Pop return address from stack
        uint8_t lowByte = memory->memory[cpu->SP];
        cpu->SP++;
        uint8_t highByte = memory->memory[cpu->SP];
        cpu->SP++;
        uint16_t returnAddress = ((uint16_t)highByte << 8) | (uint16_t)lowByte;

        // Set PC to return address
        cpu->PC = returnAddress;

        return true;
    }
};

// loadZEXALL loads the zexall.com file into memory at address 0x100
bool loadZEXALL(Memory *memory, const std::string &filename)
{
    // Open file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // Seek to end to get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate buffer and read file
    uint8_t *buffer = new uint8_t[fileSize];
    file.read(reinterpret_cast<char *>(buffer), fileSize);
    file.close();
    memory->canWriteRom = true;
    // Load into memory at address 0x100
    uint16_t loadAddress = 0x100;
    for (size_t i = 0; i < fileSize && loadAddress + i < 0x10000; i++)
    {
        memory->WriteByte(loadAddress + i, buffer[i]);
    }

    // Clean up
    delete[] buffer;

    return true;
}

// TestZEXALL runs the ZEXALL test suite
void TestZEXALL()
{
    std::cout << "ZEXALL test started" << std::endl;

    // Create memory and IO instances
    ExtendedMemory *memory = new ExtendedMemory();
    Z80 *cpu = new Z80(memory, nullptr);

    // Set up initial state for CP/M program
    // Stack pointer typically starts at 0xFFFF in CP/M programs
    cpu->SP = 0xFFFF;

    // Program counter starts at 0x100 for .COM files
    cpu->PC = 0x100;

    cpu->isNMOS = true;

    // Load zexall.com file
    if (!loadZEXALL(memory, "testdata/zexall.com"))
    {
        std::cerr << "Error: Could not load zexall.com" << std::endl;
        delete cpu;
        delete memory;
        return;
    }

    // Execute instructions
    int instructionCount = 0;
    while (true)
    {
        // Check if program has ended (PC = 0x0000)
        if (cpu->PC == 0x0000)
        {
            std::cout << "Program ended (PC reached 0x0000)" << std::endl;
            break;
        }

        // Check if this is a BDOS call (PC = 0x0005)
        if (cpu->PC == 0x0005)
        {
            // BDOS is called by jumping to 0x0005
            // Function number is in register C
            // Parameters are in other registers depending on the function

            uint8_t function = cpu->C;

            switch (function)
            {
            case 2: // Print character (character in E)
                std::cout << (char)cpu->E;
                std::cout.flush();
                break;
            case 9: // Print string (string address in DE)
            {
                uint16_t addr = (cpu->D << 8) | cpu->E; // Get DE register value
                // Read characters from memory until we encounter '$'
                while (true)
                {
                    uint8_t ch = memory->ReadByte(addr);
                    if (ch == '$')
                    {
                        break;
                    }
                    std::cout << (char)ch;
                    addr++;
                }
                std::cout.flush();
            }
            break;
            default:
                // Unknown BDOS function, just continue
                break;
            }

            // In a real CP/M system, after handling the BDOS call,
            // execution would return to the caller via a RET instruction.
            // We simulate this by popping the return address from the stack
            // and setting the PC to that address.

            // Pop return address from stack
            uint8_t lowByte = memory->memory[cpu->SP];
            cpu->SP++;
            uint8_t highByte = memory->memory[cpu->SP];
            cpu->SP++;
            uint16_t returnAddress = ((uint16_t)highByte << 8) | (uint16_t)lowByte;

            // Set PC to return address
            cpu->PC = returnAddress;

            // Continue execution
            continue;
        }

        // Execute one instruction
        int ticks = cpu->ExecuteOneInstruction();
        if (ticks <= 0)
        {
            std::cerr << "Invalid tick count: " << ticks << std::endl;
            break;
        }
        instructionCount++;

        // // Safety counter to prevent infinite loops during development
        // if (instructionCount > 10000000) {
        //     std::cout << "Safety counter reached - stopping execution" << std::endl;
        //     break;
        // }
    }

    std::cout << "Executed " << instructionCount << " instructions" << std::endl;

    // Clean up
    delete cpu;
    delete memory;
}

int main(int argc, char *argv[])
{

    TestZEXALL();

    return 0;
}
