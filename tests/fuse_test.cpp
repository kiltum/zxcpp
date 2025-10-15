#include "z80.hpp"
#include "memory.hpp"
#include "port.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

struct TestCase {
    std::string name;
    // Register values: AF, BC, DE, HL, AF_, BC_, DE_, HL_, IX, IY, SP, PC, MEMPTR
    uint16_t registers[13];
    // I, R, IFF1, IFF2, IM, HALT, tstates
    uint8_t I, R;
    bool IFF1, IFF2;
    uint8_t IM;
    bool HALT;
    int tstates;
    // Memory data: address and bytes
    std::vector<std::pair<uint16_t, std::vector<uint8_t>>> memoryBlocks;
};

struct ExpectedState {
    std::string testName;
    // Register values: AF, BC, DE, HL, AF_, BC_, DE_, HL_, IX, IY, SP, PC, MEMPTR
    uint16_t registers[13];
    // I, R, IFF1, IFF2, IM, HALT
    uint8_t I, R;
    bool IFF1, IFF2;
    uint8_t IM;
    bool HALT;
};

class FuseTest {
private:
    std::vector<TestCase> testCases;
    std::vector<std::string> expectedResults;
    std::vector<ExpectedState> expectedStates;

public:
    bool parseInputFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file " << filename << std::endl;
            return false;
        }

        std::string line;
        TestCase currentTest;
        bool readingMemory = false;
        std::vector<uint8_t> currentMemoryBytes;
        uint16_t currentAddress = 0;

        while (std::getline(file, line)) {
            // Skip empty lines
            if (line.empty()) {
                if (readingMemory && !currentMemoryBytes.empty()) {
                    currentTest.memoryBlocks.push_back({currentAddress, currentMemoryBytes});
                    currentMemoryBytes.clear();
                    readingMemory = false;
                }
                continue;
            }

            if (!readingMemory) {
                // Test name
                if (currentTest.name.empty()) {
                    currentTest.name = line;
                }
                // Register values
                else if (currentTest.registers[0] == 0 && currentTest.registers[1] == 0) {
                    std::istringstream iss(line);
                    for (int i = 0; i < 13; i++) {
                        unsigned int temp;
                        iss >> std::hex >> temp;
                        currentTest.registers[i] = static_cast<uint16_t>(temp);
                    }
                }
                // I, R, IFF1, IFF2, IM, HALT, tstates
                else {
                    std::istringstream iss(line);
                    int iff1, iff2, halt;
                    unsigned int tempI, tempR, tempIM;
                    iss >> std::hex >> tempI >> std::hex >> tempR >> 
                         std::dec >> iff1 >> std::dec >> iff2 >> std::dec >> tempIM >> 
                         std::dec >> halt >> std::dec >> currentTest.tstates;
                    currentTest.I = static_cast<uint8_t>(tempI);
                    currentTest.R = static_cast<uint8_t>(tempR);
                    currentTest.IM = static_cast<uint8_t>(tempIM);
                    currentTest.IFF1 = (iff1 != 0);
                    currentTest.IFF2 = (iff2 != 0);
                    currentTest.HALT = (halt != 0);
                    
                    // Next line should be memory data
                    readingMemory = true;
                }
            } else {
                // Memory data
                std::istringstream iss(line);
                std::string token;
                iss >> token;
                
                if (token == "-1") {
                    // End of test block
                    if (!currentMemoryBytes.empty()) {
                        currentTest.memoryBlocks.push_back({currentAddress, currentMemoryBytes});
                    }
                    testCases.push_back(currentTest);
                    
                    // Reset for next test
                    currentTest = TestCase();
                    currentMemoryBytes.clear();
                    readingMemory = false;
                } else {
                    // Memory address or data
                    uint16_t value;
                    std::istringstream(token) >> std::hex >> value;
                    
                    if (currentMemoryBytes.empty()) {
                        // This is the address
                        currentAddress = value;
                    } else {
                        // This is data
                        currentMemoryBytes.push_back(static_cast<uint8_t>(value));
                    }
                }
            }
        }

        file.close();
        return true;
    }

    bool parseExpectedFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open expected file " << filename << std::endl;
            return false;
        }

        std::string line;
        ExpectedState currentExpected;
        bool readingRegisters = false;
        
        while (std::getline(file, line)) {
            // Store all lines for reference
            expectedResults.push_back(line);
            
            // Skip lines with MR, MW, PR, PW (memory read/write operations)
            if (line.find("MR ") != std::string::npos || 
                line.find("MW ") != std::string::npos ||
                line.find("PR ") != std::string::npos ||
                line.find("PW ") != std::string::npos) {
                continue;
            }
            
            // Check if this is a test name (all digits)
            bool isTestName = true;
            for (char c : line) {
                if (!std::isdigit(c) && !std::isspace(c)) {
                    isTestName = false;
                    break;
                }
            }
            
            if (isTestName && !line.empty()) {
                // This is a new test block
                if (!currentExpected.testName.empty()) {
                    // Save previous expected state
                    expectedStates.push_back(currentExpected);
                }
                
                // Start new expected state
                currentExpected = ExpectedState();
                currentExpected.testName = line;
                readingRegisters = true;
                continue;
            }
            
            // If we're reading registers and the line contains register values
            if (readingRegisters && !line.empty() && line[0] != ' ') {
                std::istringstream iss(line);
                std::string token;
                
                // Try to parse register values
                if (iss >> std::hex >> currentExpected.I >> std::hex >> currentExpected.R) {
                    int iff1, iff2, halt;
                    unsigned int tempIM;
                    if (iss >> std::dec >> iff1 >> std::dec >> iff2 >> std::dec >> tempIM >> std::dec >> halt) {
                        currentExpected.IM = static_cast<uint8_t>(tempIM);
                        currentExpected.IFF1 = (iff1 != 0);
                        currentExpected.IFF2 = (iff2 != 0);
                        currentExpected.HALT = (halt != 0);
                        readingRegisters = false;
                        continue;
                    }
                }
                
                // Try to parse main registers (AF, BC, DE, HL, AF_, BC_, DE_, HL_, IX, IY, SP, PC, MEMPTR)
                std::istringstream regIss(line);
                bool parsedRegisters = true;
                for (int i = 0; i < 13; i++) {
                    if (!(regIss >> std::hex >> currentExpected.registers[i])) {
                        parsedRegisters = false;
                        break;
                    }
                }
                
                if (parsedRegisters) {
                    continue;
                }
            }
        }
        
        // Save the last expected state
        if (!currentExpected.testName.empty()) {
            expectedStates.push_back(currentExpected);
        }

        file.close();
        return true;
    }

    void initializeCPU(Z80& cpu, Memory& memory, Port& port, const TestCase& test) {
        // Initialize registers
        cpu.AF = test.registers[0];
        cpu.BC = test.registers[1];
        cpu.DE = test.registers[2];
        cpu.HL = test.registers[3];
        cpu.AF_ = test.registers[4];
        cpu.BC_ = test.registers[5];
        cpu.DE_ = test.registers[6];
        cpu.HL_ = test.registers[7];
        cpu.IX = test.registers[8];
        cpu.IY = test.registers[9];
        cpu.SP = test.registers[10];
        cpu.PC = test.registers[11];
        cpu.MEMPTR = test.registers[12];
        
        // Initialize internal registers and flags
        cpu.I = test.I;
        cpu.R = test.R;
        cpu.IFF1 = test.IFF1;
        cpu.IFF2 = test.IFF2;
        cpu.IM = test.IM;
        cpu.HALT = test.HALT;
        
        // Initialize memory
        for (const auto& block : test.memoryBlocks) {
            uint16_t addr = block.first;
            for (size_t i = 0; i < block.second.size(); i++) {
                memory.WriteByte(addr + i, block.second[i]);
            }
        }
    }

    void printRegisterState(const std::string& label, const uint16_t registers[13], 
                           uint8_t I, uint8_t R, bool IFF1, bool IFF2, uint8_t IM, bool HALT) {
        std::cout << label << ": ";
        for (int i = 0; i < 13; i++) {
            std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << registers[i] << " ";
        }
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(I) << " "
                  << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(R) << " "
                  << IFF1 << " " << IFF2 << " " << std::dec << static_cast<int>(IM) << " " << HALT << std::endl;
        std::cout << std::dec; // Reset to decimal to avoid affecting other output
    }

    bool compareResults(const TestCase& test, const Z80& cpu) {
        // Find expected state for this test
        auto it = std::find_if(expectedStates.begin(), expectedStates.end(),
                              [&test](const ExpectedState& es) { return es.testName == test.name; });
        
        if (it == expectedStates.end()) {
            std::cout << "  ERROR: Expected state not found for test " << test.name << std::endl;
            return false;
        }
        
        const ExpectedState& expected = *it;
        
        // Collect actual state from CPU
        uint16_t actualRegisters[13] = {
            cpu.AF, cpu.BC, cpu.DE, cpu.HL,
            cpu.AF_, cpu.BC_, cpu.DE_, cpu.HL_,
            cpu.IX, cpu.IY, cpu.SP, cpu.PC, cpu.MEMPTR
        };
        
        // Register names for display
        const char* registerNames[13] = {
            "AF", "BC", "DE", "HL", "AF'", "BC'", "DE'", "HL'", "IX", "IY", "SP", "PC", "MEMPTR"
        };
        
        // Compare registers and collect differences
        std::vector<std::string> differences;
        for (int i = 0; i < 13; i++) {
            if (actualRegisters[i] != expected.registers[i]) {
                std::stringstream actualStr, expectedStr;
                actualStr << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << actualRegisters[i];
                expectedStr << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << expected.registers[i];
                differences.push_back(std::string(registerNames[i]) + "(" + 
                                    actualStr.str() + "!=" + 
                                    expectedStr.str() + ")");
            }
        }
        
        // Compare internal registers and flags
        if (cpu.I != expected.I) {
            std::stringstream actualStr, expectedStr;
            actualStr << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)cpu.I;
            expectedStr << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)expected.I;
            differences.push_back(std::string("I(") + 
                                actualStr.str() + "!=" + 
                                expectedStr.str() + ")");
        }
        if (cpu.R != expected.R) {
            std::stringstream actualStr, expectedStr;
            actualStr << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)cpu.R;
            expectedStr << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)expected.R;
            differences.push_back(std::string("R(") + 
                                actualStr.str() + "!=" + 
                                expectedStr.str() + ")");
        }
        if (cpu.IFF1 != expected.IFF1) {
            differences.push_back(std::string("IFF1(") + 
                                (cpu.IFF1 ? "1" : "0") + "!=" + 
                                (expected.IFF1 ? "1" : "0") + ")");
        }
        if (cpu.IFF2 != expected.IFF2) {
            differences.push_back(std::string("IFF2(") + 
                                (cpu.IFF2 ? "1" : "0") + "!=" + 
                                (expected.IFF2 ? "1" : "0") + ")");
        }
        if (cpu.IM != expected.IM) {
            std::stringstream actualStr, expectedStr;
            actualStr << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)cpu.IM;
            expectedStr << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)expected.IM;
            differences.push_back(std::string("IM(") + 
                                actualStr.str() + "!=" + 
                                expectedStr.str() + ")");
        }
        if (cpu.HALT != expected.HALT) {
            differences.push_back(std::string("HALT(") + 
                                (cpu.HALT ? "1" : "0") + "!=" + 
                                (expected.HALT ? "1" : "0") + ")");
        }
        
        // If no differences, return true
        if (differences.empty()) {
            return true; // All match
        }
        
        // Print differences with register names
        std::cout << "  Register differences found: ";
        for (size_t i = 0; i < differences.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << differences[i];
        }
        std::cout << std::endl;
        
        // Print three-line comparison
        std::cout << "            AF   BC   DE   HL   AF'  BC'  DE'  HL'  IX   IY   SP   PC   MEM  IM R  1 2 I H" << std::endl;
        printRegisterState("  Initial ", test.registers, test.IM, test.R, test.IFF1, test.IFF2, test.I, test.HALT);
        printRegisterState("  Expected", expected.registers, expected.IM, expected.R, expected.IFF1, expected.IFF2, expected.I, expected.HALT);
        printRegisterState("  Actual  ", actualRegisters, cpu.IM, cpu.R, cpu.IFF1, cpu.IFF2, cpu.I, cpu.HALT);
        
        return false;
    }

    bool runTest(const TestCase& test) {
        std::cout << "Running test: " << test.name << std::endl;
        
        // Create instances
        Memory memory;
        Port port;
        Z80 cpu(&memory);
        
        // Initialize CPU state
        initializeCPU(cpu, memory, port, test);
        
        // Save initial state for reporting
        uint16_t initialRegisters[13] = {
            cpu.AF, cpu.BC, cpu.DE, cpu.HL,
            cpu.AF_, cpu.BC_, cpu.DE_, cpu.HL_,
            cpu.IX, cpu.IY, cpu.SP, cpu.PC, cpu.MEMPTR
        };
        uint8_t initialI = cpu.I;
        uint8_t initialR = cpu.R;
        bool initialIFF1 = cpu.IFF1;
        bool initialIFF2 = cpu.IFF2;
        uint8_t initialIM = cpu.IM;
        bool initialHALT = cpu.HALT;
        
        // Execute instructions until we've consumed enough tstates
        int totalTStates = 0;
        while (totalTStates < test.tstates) {
            int tstates = cpu.ExecuteOneInstruction();
            if (tstates <= 0) {
                // If ExecuteOneInstruction returns 0 or negative, we can't continue
                break;
            }
            totalTStates += tstates;
        }
        
        // Compare results
        return compareResults(test, cpu);
    }

    void runAllTests() {
        std::cout << "Running " << testCases.size() << " tests..." << std::endl;
        
        int passed = 0;
        int failed = 0;
        
        for (const auto& test : testCases) {
            if (runTest(test)) {
                passed++;
                std::cout << "  PASSED" << std::endl;
            } else {
                failed++;
                std::cout << "  FAILED" << std::endl;
            }
        }
        
        std::cout << "Tests completed: " << passed << " passed, " << failed << " failed" << std::endl;
    }
};

int main() {
    FuseTest tester;
    
    // Parse input file
    if (!tester.parseInputFile("tests/testdata/tests.in")) {
        return 1;
    }
    
    // Parse expected file
    if (!tester.parseExpectedFile("tests/testdata/tests.expected")) {
        return 1;
    }
    
    // Run all tests
    tester.runAllTests();
    
    return 0;
}
