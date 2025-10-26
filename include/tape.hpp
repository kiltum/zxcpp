#ifndef TAPE_HPP
#define TAPE_HPP

#include <cstdint>
#include <string>
#include <vector>

// Structure to represent a TAP block
struct TapBlock {
    uint16_t length;           // Length of the block (including flag and checksum)
    uint8_t flag;              // Flag byte (0x00 for headers, 0xFF for data blocks)
    std::vector<uint8_t> data; // Data bytes (without length, flag, and checksum)
    uint8_t checksum;          // Checksum byte
    bool isValid;              // Whether the checksum is valid
    
    // Header-specific fields (only valid for header blocks)
    uint8_t fileType;          // Type of file (0=Program, 1=Number array, 2=Character array, 3=Bytes)
    std::string filename;      // Filename (10 characters, padded with spaces)
    uint16_t dataLength;       // Length of the data block
    uint16_t param1;           // Parameter 1 (depends on file type)
    uint16_t param2;           // Parameter 2 (depends on file type)
};

// Structure to represent a bit stream impulse
struct TapeImpulse {
    uint32_t ticks;  // Duration in ticks
    bool value;      // Signal value (true/false)
};

class Tape {
private:
    std::vector<uint8_t> tapeData;
    std::vector<TapBlock> tapBlocks; // Store parsed TAP blocks
    std::vector<TapeImpulse> bitStream; // Store generated bit stream
    size_t currentImpulseIndex; // Index of current impulse in bit stream
    uint32_t currentImpulseTicks; // Ticks elapsed in current impulse
    
    // Helper function to validate checksum
    bool validateChecksum(const std::vector<uint8_t>& blockData);
    
    // Helper function to parse header information
    void parseHeaderInfo(TapBlock& block);

    uint tapePilotLenHeader; // How many impulses in pilot tone for header
    uint tapePilotLenData;   // and for data
    uint tapePilot;          // Lenght in ticks how many ticks in one impulse
    uint tapeSync1;          // length of sync impulses
    uint tapeSync2;
    uint tapePilotPause;     // lenght in ticks pause between blocks 
    uint tape0;              // length in ticks 0 bit
    uint tape1;              // length in ticks 1 bit
    
public:
    // Constructor
    Tape();
    
    // Destructor
    ~Tape();
    
    // Reset tape state
    void reset();
    
    // Load tape file
    bool loadFile(const std::string& fileName);
    
    // Load virtual tape data directly
    void loadVirtualTape(const std::vector<uint8_t>& data);
    
    // Parse TAP file format
    void parseTap(const std::vector<uint8_t>& data);
    
    // Parse TZX file format
    void parseTzx(const std::vector<uint8_t>& data);
    
    // Prepare bit stream from parsed blocks
    void prepareBitStream();
    
    // Get number of parsed blocks
    size_t getBlockCount() const;
    
    // Get a specific block
    const TapBlock& getBlock(size_t index) const;
    
    // Get the bit stream for debugging
    const std::vector<TapeImpulse>& getBitStream() const;
    
    // For testing purposes: set up a test bit stream
    void setTestBitStream(const std::vector<TapeImpulse>& testStream);
    
    bool isTapePlayed;
    bool getNextBit();
};

#endif // TAPE_HPP
