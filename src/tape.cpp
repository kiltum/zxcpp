#include "tape.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <string>
#include <zip.h>

// Constructor
Tape::Tape()
{
    reset();
}

// Destructor
Tape::~Tape()
{
    // Nothing to clean up
}

// Reset tape state
void Tape::reset()
{
    isTapePlayed = false;
    tapeData.clear();
    tapBlocks.clear();
    bitStream.clear();
    currentImpulseIndex = 0;
    currentImpulseTicks = 0;
    tapePilotLenHeader=3000;//8063;
    tapePilotLenData=3223;
    tapePilot = 2168;
    tapePilotPause=3500000;
    tape0 = 855;
    tape1 = 1710; 
    tapeSync1 = 667;
    tapeSync2 = 735;
    tapeFinalSync = 945;
}

// Helper function to check if a string ends with a specific suffix
bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

// Helper function to validate checksum
bool Tape::validateChecksum(const std::vector<uint8_t>& blockData)
{
    if (blockData.size() < 2) {
        return false;
    }
    
    uint8_t calculatedChecksum = 0;
    for (size_t i = 0; i < blockData.size() - 1; i++) {
        calculatedChecksum ^= blockData[i];
    }
    
    return calculatedChecksum == blockData.back();
}

// Helper function to parse header information
void Tape::parseHeaderInfo(TapBlock& block)
{
    if (block.flag != 0x00 || block.data.size() < 17) {
        // Not a header block or insufficient data
        return;
    }
    
    // Parse header fields
    block.fileType = block.data[0];
    
    // Extract filename (10 characters)
    block.filename = std::string(reinterpret_cast<const char*>(block.data.data() + 1), 10);
    
    // Extract data length (2 bytes, little-endian)
    block.dataLength = static_cast<uint16_t>(block.data[11]) | 
                      (static_cast<uint16_t>(block.data[12]) << 8);
    
    // Extract parameters (2 bytes each, little-endian)
    block.param1 = static_cast<uint16_t>(block.data[13]) | 
                  (static_cast<uint16_t>(block.data[14]) << 8);
    
    block.param2 = static_cast<uint16_t>(block.data[15]) | 
                  (static_cast<uint16_t>(block.data[16]) << 8);
}

// Load tape file
bool Tape::loadFile(const std::string& fileName)
{
    // Convert filename to lowercase for comparison
    std::string lowerFileName = fileName;
    std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);
    
    // Check if file is zipped
    if (endsWith(lowerFileName, ".zip")) {
        std::cout << "ZIP file detected: " << fileName << std::endl;
        
        // Open ZIP archive
        int err = 0;
        zip_t* archive = zip_open(fileName.c_str(), 0, &err);
        if (!archive) {
            std::cerr << "Failed to open ZIP file: " << fileName << " (error: " << err << ")" << std::endl;
            return false;
        }
        
        // Get the number of entries in the ZIP
        zip_int64_t num_entries = zip_get_num_entries(archive, 0);
        if (num_entries <= 0) {
            std::cerr << "ZIP file is empty: " << fileName << std::endl;
            zip_close(archive);
            return false;
        }
        
        // Look for the first file with .tap or .tzx extension
        zip_int64_t target_index = -1;
        std::string target_filename;
        bool is_tap = false;
        
        for (zip_int64_t i = 0; i < num_entries; i++) {
            const char* entry_name = zip_get_name(archive, i, 0);
            if (!entry_name) continue;
            
            std::string entry_name_str(entry_name);
            std::transform(entry_name_str.begin(), entry_name_str.end(), entry_name_str.begin(), ::tolower);
            
            if (endsWith(entry_name_str, ".tap")) {
                target_index = i;
                target_filename = entry_name;
                is_tap = true;
                break;
            } else if (endsWith(entry_name_str, ".tzx")) {
                target_index = i;
                target_filename = entry_name;
                is_tap = false;
                break;
            }
        }
        
        if (target_index == -1) {
            std::cerr << "No supported tape file (.tap or .tzx) found in ZIP: " << fileName << std::endl;
            zip_close(archive);
            return false;
        }
        
        // Open the target file from ZIP
        zip_file_t* file = zip_fopen_index(archive, target_index, 0);
        if (!file) {
            std::cerr << "Failed to open file from ZIP: " << target_filename << std::endl;
            zip_close(archive);
            return false;
        }
        
        // Read the file data
        std::vector<uint8_t> data;
        char buffer[8192];
        zip_int64_t bytes_read;
        
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            data.insert(data.end(), buffer, buffer + bytes_read);
        }
        
        // Close the file and archive
        zip_fclose(file);
        zip_close(archive);
        
        if (bytes_read < 0) {
            std::cerr << "Error reading file from ZIP: " << target_filename << std::endl;
            return false;
        }
        
        std::cout << "Extracted " << data.size() << " bytes from " << target_filename << std::endl;
        
        // Parse the extracted data
        if (is_tap) {
            parseTap(data);
        } else {
            parseTzx(data);
        }
        
        return true;
    }
    
    // Handle non-zipped files
    if (endsWith(lowerFileName, ".tap")) {
        // Read file data
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << fileName << std::endl;
            return false;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        tapeData.resize(size);
        if (!file.read(reinterpret_cast<char*>(tapeData.data()), size)) {
            std::cerr << "Failed to read file: " << fileName << std::endl;
            return false;
        }
        
        parseTap(tapeData);
        return true;
    } else if (endsWith(lowerFileName, ".tzx")) {
        // Read file data
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << fileName << std::endl;
            return false;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        tapeData.resize(size);
        if (!file.read(reinterpret_cast<char*>(tapeData.data()), size)) {
            std::cerr << "Failed to read file: " << fileName << std::endl;
            return false;
        }
        
        parseTzx(tapeData);
        return true;
    }
    
    std::cerr << "Unsupported file format: " << fileName << std::endl;
    return false;
}

// Load virtual tape data directly
void Tape::loadVirtualTape(const std::vector<uint8_t>& data) {
    std::cout << "Loading virtual tape with " << data.size() << " bytes" << std::endl;
    tapeData = data;
    parseTap(tapeData);
}

// Parse TAP file format
void Tape::parseTap(const std::vector<uint8_t>& data)
{
    //std::cout << "Parsing TAP file with " << data.size() << " bytes" << std::endl;
    
    // Clear any existing blocks
    tapBlocks.clear();
    
    size_t pos = 0;
    while (pos + 2 <= data.size()) {
        // Read block length (2 bytes, little-endian)
        uint16_t blockLength = static_cast<uint16_t>(data[pos]) | 
                              (static_cast<uint16_t>(data[pos + 1]) << 8);
        
        // Check if we have enough data for the complete block
        if (pos + 2 + blockLength > data.size()) {
            std::cerr << "Incomplete block at position " << pos << std::endl;
            break;
        }
        
        // Create a new block
        TapBlock block;
        block.length = blockLength;
        
        // Extract all data (including flag and checksum)
        if (blockLength > 0) {
            block.data.assign(data.begin() + pos + 2, data.begin() + pos + 2 + blockLength);
        }
        
        // Extract flag byte from data for compatibility
        if (block.data.size() > 0) {
            block.flag = block.data[0];
        }
        
        // Extract checksum from data for compatibility
        if (block.data.size() > 0) {
            block.checksum = block.data.back();
        }
        
        // Validate checksum
        block.isValid = validateChecksum(block.data);
        
        // Parse header information if this is a header block
        if (block.flag == 0x00 && block.data.size() >= 18) {  // Need at least 18 bytes for header (flag + 17 header bytes)
            // For header blocks, we need to parse the header info from data[1] to data[17]
            // We temporarily modify the block to match the expected format for parseHeaderInfo
            std::vector<uint8_t> tempData(block.data.begin() + 1, block.data.end() - 1);  // Exclude flag and checksum
            TapBlock tempBlock = block;
            tempBlock.data = tempData;
            parseHeaderInfo(tempBlock);
            // Copy back the parsed header info
            block.fileType = tempBlock.fileType;
            block.filename = tempBlock.filename;
            block.dataLength = tempBlock.dataLength;
            block.param1 = tempBlock.param1;
            block.param2 = tempBlock.param2;
        }
        
        // Add block to our collection
        tapBlocks.push_back(block);
        
        // Move to next block
        pos += 2 + blockLength;
        
        // std::cout << "  Parsed block: length=" << blockLength 
        //           << ", flag=0x" << std::hex << static_cast<int>(block.flag) << std::dec
        //           << ", data_size=" << block.data.size() 
        //           << ", checksum_valid=" << (block.isValid ? "yes" : "no") << std::endl;
    }
    
    std::cout << "Parsed " << tapBlocks.size() << " blocks from TAP file" << std::endl;
}

// Parse TZX file format
void Tape::parseTzx(const std::vector<uint8_t>& data)
{
    // Empty implementation - to be filled later
    std::cout << "Parsing TZX file with " << data.size() << " bytes" << std::endl;
}

// Get number of parsed blocks
size_t Tape::getBlockCount() const
{
    return tapBlocks.size();
}

// Get a specific block
const TapBlock& Tape::getBlock(size_t index) const
{
    static TapBlock emptyBlock; // Return empty block if index is out of bounds
    if (index >= tapBlocks.size()) {
        return emptyBlock;
    }
    return tapBlocks[index];
}

// Get the bit stream for debugging
const std::vector<TapeImpulse>& Tape::getBitStream() const {
    return bitStream;
}

// For testing purposes: set up a test bit stream
void Tape::setTestBitStream(const std::vector<TapeImpulse>& testStream) {
    bitStream = testStream;
    currentImpulseIndex = 0;
    currentImpulseTicks = 0;
}

// Prepare bit stream from parsed blocks
// This function generates a byte stream where each impulse is represented as (uint32_t ticks, bool value)
void Tape::prepareBitStream()
{
    // Clear any existing bit stream
    bitStream.clear();
    
    // Process each block and generate the corresponding impulses
    for (size_t i = 0; i < tapBlocks.size(); ++i) {
        const TapBlock& block = tapBlocks[i];
        
        // Determine if this is a header or data block to set appropriate pilot tone length
        uint pilotLength = (block.flag == 0x00) ? tapePilotLenHeader : tapePilotLenData;
        
        // Generate pilot tone
        // For each impulse: value=1 for tapePilot ticks, then value=0 for tapePilot ticks
       
        for (uint j = 0; j < pilotLength; ++j) {
            // High impulse
            TapeImpulse highImpulse;
            highImpulse.ticks = tapePilot;
            highImpulse.value = true;
            bitStream.push_back(highImpulse);
            
            // Low impulse
            TapeImpulse lowImpulse;
            lowImpulse.ticks = tapePilot;
            lowImpulse.value = false;
            bitStream.push_back(lowImpulse);
        }
        
        // Generate sync pulses
        // First sync pulse: tapeSync1 ticks with value=1
        TapeImpulse sync1Impulse;
        sync1Impulse.ticks = tapeSync1;
        sync1Impulse.value = true;
        bitStream.push_back(sync1Impulse);
        
        // Second sync pulse: tapeSync2 ticks with value=0
        TapeImpulse sync2Impulse;
        sync2Impulse.ticks = tapeSync2;
        sync2Impulse.value = false;
        bitStream.push_back(sync2Impulse);
        
        // Generate data bits
        // For each byte in the block (including flag, data, and checksum):
        //   - For each bit (MSB first):
        //     - Generate bit impulse (tape0 for 0, tape1 for 1)
        //     - Each impulse consists of value=1 then value=0
        
        // Process each byte directly from block.data
        for (size_t byteIndex = 0; byteIndex < block.data.size(); ++byteIndex) {
            uint8_t byte = block.data[byteIndex];
            //printf("Parse %d %zu\n", i,byteIndex);
            // Process each bit (MSB first)
            for (int bitIndex = 7; bitIndex >= 0; --bitIndex)  {
            //for (int bitIndex = 0; bitIndex <7; bitIndex++)  {
                bool bitValue = (byte >> bitIndex) & 1;
                
                // Determine pulse length based on bit value
                uint pulseLength = bitValue ? tape1 : tape0;
                
                // Generate impulse: value=1 for pulseLength ticks
                TapeImpulse highImpulse;
                highImpulse.ticks = pulseLength;
                highImpulse.value = true;
                bitStream.push_back(highImpulse);
                
                // Generate impulse: value=0 for pulseLength ticks
                TapeImpulse lowImpulse;
                lowImpulse.ticks = pulseLength;
                lowImpulse.value = false;
                bitStream.push_back(lowImpulse);
            }
        }
        
        sync2Impulse.ticks = tapeFinalSync;
        sync2Impulse.value = true;
        bitStream.push_back(sync2Impulse); 
        // Generate pause between blocks
        // Pause is all 0 for tapePilotPause length
        TapeImpulse pauseImpulse;
        pauseImpulse.ticks = tapePilotPause;
        pauseImpulse.value = false;
        bitStream.push_back(pauseImpulse);
    }
    printf("Consumed %lu bytes for bitStream\n", bitStream.size() * sizeof(TapeImpulse));
}

// Get next audio input state for ULA
bool Tape::getNextBit()
{   
    // If no tape played, return 
    if(!isTapePlayed) return false;
    // If no bit stream has been generated, return false
    if (bitStream.empty()) {
        isTapePlayed = false;
        return false;
    }
    
    // If we've processed all impulses, return false (pause state)
    if (currentImpulseIndex >= bitStream.size()) {
        isTapePlayed = false;
        return false;
    }
    
    // Get the current impulse
    const TapeImpulse& currentImpulse = bitStream[currentImpulseIndex];
    
    // Increment the tick counter for the current impulse
    currentImpulseTicks++;
    
    // Check if we've exhausted the current impulse
    if (currentImpulseTicks >= currentImpulse.ticks) {
        // Move to the next impulse
        currentImpulseIndex++;
        currentImpulseTicks = 0;
    }
    //printf("%d ",currentImpulseIndex);
    // Return the value of the current impulse
    return currentImpulse.value;
}
