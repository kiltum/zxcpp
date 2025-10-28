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
    std::cout << "Parsing TZX file with " << data.size() << " bytes" << std::endl;
    
    // Clear any existing blocks
    tapBlocks.clear();
    
    // Check header
    if (data.size() < 10) {
        std::cerr << "TZX file too small" << std::endl;
        return;
    }
    
    // Check signature "ZXTape!"
    if (memcmp(data.data(), "ZXTape!", 7) != 0 || data[7] != 0x1A) {
        std::cerr << "Invalid TZX signature" << std::endl;
        return;
    }
    
    // Check version (we support 1.x)
    uint8_t majorVersion = data[8];
    uint8_t minorVersion = data[9];
    if (majorVersion != 1) {
        std::cerr << "Unsupported TZX version: " << (int)majorVersion << "." << (int)minorVersion << std::endl;
        return;
    }
    
    std::cout << "TZX version " << (int)majorVersion << "." << (int)minorVersion << std::endl;
    
    // Parse blocks
    size_t pos = 10; // Start after header
    while (pos < data.size()) {
        if (pos + 1 > data.size()) {
            std::cerr << "Unexpected end of file" << std::endl;
            break;
        }
        
        uint8_t blockId = data[pos];
        pos++;
        
        switch (blockId) {
            case 0x10: // Standard speed data block
                pos = parseTzxStandardSpeedBlock(data, pos);
                break;
                
            case 0x11: // Turbo speed data block
                pos = parseTzxTurboSpeedBlock(data, pos);
                break;
                
            case 0x12: // Pure tone
                pos = parseTzxPureToneBlock(data, pos);
                break;
                
            case 0x13: // Sequence of pulses of various lengths
                pos = parseTzxPulseSequenceBlock(data, pos);
                break;
                
            case 0x14: // Pure data block
                pos = parseTzxPureDataBlock(data, pos);
                break;
                
            case 0x15: // Direct recording block
                pos = parseTzxDirectRecordingBlock(data, pos);
                break;
                
            case 0x20: // Pause (silence) or 'Stop the tape'
                pos = parseTzxPauseBlock(data, pos);
                break;
                
            case 0x21: // Group start
                pos = parseTzxGroupStartBlock(data, pos);
                break;
                
            case 0x22: // Group end
                pos = parseTzxGroupEndBlock(data, pos);
                break;
                
            case 0x23: // Jump to block
                pos = parseTzxJumpBlock(data, pos);
                break;
                
            case 0x24: // Loop start
                pos = parseTzxLoopStartBlock(data, pos);
                break;
                
            case 0x25: // Loop end
                pos = parseTzxLoopEndBlock(data, pos);
                break;
                
            case 0x26: // Call sequence
                pos = parseTzxCallSequenceBlock(data, pos);
                break;
                
            case 0x27: // Return from sequence
                pos = parseTzxReturnSequenceBlock(data, pos);
                break;
                
            case 0x28: // Select block
                pos = parseTzxSelectBlock(data, pos);
                break;
                
            case 0x2A: // Stop the tape if in 48K mode
                pos = parseTzxStop48KBlock(data, pos);
                break;
                
            case 0x2B: // Set signal level
                pos = parseTzxSetLevelBlock(data, pos);
                break;
                
            case 0x30: // Text description
                pos = parseTzxTextDescriptionBlock(data, pos);
                break;
                
            case 0x31: // Message block
                pos = parseTzxMessageBlock(data, pos);
                break;
                
            case 0x32: // Archive info
                pos = parseTzxArchiveInfoBlock(data, pos);
                break;
                
            case 0x33: // Hardware type
                pos = parseTzxHardwareTypeBlock(data, pos);
                break;
                
            case 0x35: // Custom info block
                pos = parseTzxCustomInfoBlock(data, pos);
                break;
                
            case 0x5A: // Glue block
                pos = parseTzxGlueBlock(data, pos);
                break;
                
            default:
                std::cerr << "Unknown TZX block ID: 0x" << std::hex << (int)blockId << std::dec << std::endl;
                // Try to skip this block by reading its length
                if (pos + 4 <= data.size()) {
                    uint32_t blockLength = static_cast<uint32_t>(data[pos]) |
                                          (static_cast<uint32_t>(data[pos + 1]) << 8) |
                                          (static_cast<uint32_t>(data[pos + 2]) << 16) |
                                          (static_cast<uint32_t>(data[pos + 3]) << 24);
                    pos += 4 + blockLength;
                } else {
                    std::cerr << "Unable to skip unknown block, file may be corrupt" << std::endl;
                    return;
                }
                break;
        }
        
        if (pos > data.size()) {
            std::cerr << "Block parsing went beyond file end" << std::endl;
            break;
        }
    }
    
    std::cout << "Parsed " << tapBlocks.size() << " blocks from TZX file" << std::endl;
}

// Parse TZX Standard Speed Data Block (ID 10)
size_t Tape::parseTzxStandardSpeedBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 4 > data.size()) {
        std::cerr << "Incomplete TZX Standard Speed Data Block" << std::endl;
        return data.size();
    }
    
    // Read pause duration (2 bytes)
    uint16_t pauseDuration = static_cast<uint16_t>(data[pos]) | 
                            (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read data length (2 bytes)
    uint16_t dataLength = static_cast<uint16_t>(data[pos]) | 
                         (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Check if we have enough data
    if (pos + dataLength > data.size()) {
        std::cerr << "Incomplete TZX Standard Speed Data Block data" << std::endl;
        return data.size();
    }
    
    // Create a new TapBlock
    TapBlock block;
    block.length = dataLength;
    
    // Extract all data (including flag and checksum)
    if (dataLength > 0) {
        block.data.assign(data.begin() + pos, data.begin() + pos + dataLength);
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
    
    // Move position past the data
    pos += dataLength;
    
    return pos;
}

// Parse TZX Turbo Speed Data Block (ID 11)
size_t Tape::parseTzxTurboSpeedBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 18 > data.size()) {
        std::cerr << "Incomplete TZX Turbo Speed Data Block header" << std::endl;
        return data.size();
    }
    
    // Read all the timing parameters
    uint16_t pilotPulseLength = static_cast<uint16_t>(data[pos]) | 
                               (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    uint16_t sync1PulseLength = static_cast<uint16_t>(data[pos]) | 
                               (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    uint16_t sync2PulseLength = static_cast<uint16_t>(data[pos]) | 
                               (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    uint16_t zeroBitPulseLength = static_cast<uint16_t>(data[pos]) | 
                                 (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    uint16_t oneBitPulseLength = static_cast<uint16_t>(data[pos]) | 
                                (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    uint16_t pilotToneLength = static_cast<uint16_t>(data[pos]) | 
                              (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    uint8_t usedBitsInLastByte = data[pos];
    pos += 1;
    
    uint16_t pauseDuration = static_cast<uint16_t>(data[pos]) | 
                            (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read data length (3 bytes)
    uint32_t dataLength = static_cast<uint32_t>(data[pos]) |
                         (static_cast<uint32_t>(data[pos + 1]) << 8) |
                         (static_cast<uint32_t>(data[pos + 2]) << 16);
    pos += 3;
    
    // Check if we have enough data
    if (pos + dataLength > data.size()) {
        std::cerr << "Incomplete TZX Turbo Speed Data Block data" << std::endl;
        return data.size();
    }
    
    // Create a new TapBlock
    TapBlock block;
    block.length = static_cast<uint16_t>(dataLength & 0xFFFF); // Truncate to 16-bit for compatibility
    
    // Extract all data (including flag and checksum)
    if (dataLength > 0) {
        block.data.assign(data.begin() + pos, data.begin() + pos + dataLength);
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
    
    // Move position past the data
    pos += dataLength;
    
    return pos;
}

// Parse TZX Pure Tone Block (ID 12)
size_t Tape::parseTzxPureToneBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 4 > data.size()) {
        std::cerr << "Incomplete TZX Pure Tone Block" << std::endl;
        return data.size();
    }
    
    // Read pulse length (2 bytes)
    uint16_t pulseLength = static_cast<uint16_t>(data[pos]) | 
                          (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read number of pulses (2 bytes)
    uint16_t pulseCount = static_cast<uint16_t>(data[pos]) | 
                         (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Pure Tone Block: " << pulseCount << " pulses of " << pulseLength << " T-states" << std::endl;
    
    return pos;
}

// Parse TZX Pulse Sequence Block (ID 13)
size_t Tape::parseTzxPulseSequenceBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 1 > data.size()) {
        std::cerr << "Incomplete TZX Pulse Sequence Block" << std::endl;
        return data.size();
    }
    
    // Read number of pulses
    uint8_t pulseCount = data[pos];
    pos += 1;
    
    // Check if we have enough data for all pulses
    if (pos + (2 * pulseCount) > data.size()) {
        std::cerr << "Incomplete TZX Pulse Sequence Block data" << std::endl;
        return data.size();
    }
    
    // Skip all pulse lengths (2 bytes each)
    pos += 2 * pulseCount;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Pulse Sequence Block: " << (int)pulseCount << " pulses" << std::endl;
    
    return pos;
}

// Parse TZX Pure Data Block (ID 14)
size_t Tape::parseTzxPureDataBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 10 > data.size()) {
        std::cerr << "Incomplete TZX Pure Data Block header" << std::endl;
        return data.size();
    }
    
    // Read zero bit pulse length (2 bytes)
    uint16_t zeroBitPulseLength = static_cast<uint16_t>(data[pos]) | 
                                 (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read one bit pulse length (2 bytes)
    uint16_t oneBitPulseLength = static_cast<uint16_t>(data[pos]) | 
                                (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read used bits in last byte (1 byte)
    uint8_t usedBitsInLastByte = data[pos];
    pos += 1;
    
    // Read pause duration (2 bytes)
    uint16_t pauseDuration = static_cast<uint16_t>(data[pos]) | 
                            (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read data length (3 bytes)
    uint32_t dataLength = static_cast<uint32_t>(data[pos]) |
                         (static_cast<uint32_t>(data[pos + 1]) << 8) |
                         (static_cast<uint32_t>(data[pos + 2]) << 16);
    pos += 3;
    
    // Check if we have enough data
    if (pos + dataLength > data.size()) {
        std::cerr << "Incomplete TZX Pure Data Block data" << std::endl;
        return data.size();
    }
    
    // Create a new TapBlock
    TapBlock block;
    block.length = static_cast<uint16_t>(dataLength & 0xFFFF); // Truncate to 16-bit for compatibility
    
    // Extract all data (including flag and checksum)
    if (dataLength > 0) {
        block.data.assign(data.begin() + pos, data.begin() + pos + dataLength);
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
    
    // Move position past the data
    pos += dataLength;
    
    return pos;
}

// Parse TZX Direct Recording Block (ID 15)
size_t Tape::parseTzxDirectRecordingBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 8 > data.size()) {
        std::cerr << "Incomplete TZX Direct Recording Block header" << std::endl;
        return data.size();
    }
    
    // Read T-states per sample (2 bytes)
    uint16_t tStatesPerSample = static_cast<uint16_t>(data[pos]) | 
                               (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read pause duration (2 bytes)
    uint16_t pauseDuration = static_cast<uint16_t>(data[pos]) | 
                            (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Read used bits in last byte (1 byte)
    uint8_t usedBitsInLastByte = data[pos];
    pos += 1;
    
    // Read data length (3 bytes)
    uint32_t dataLength = static_cast<uint32_t>(data[pos]) |
                         (static_cast<uint32_t>(data[pos + 1]) << 8) |
                         (static_cast<uint32_t>(data[pos + 2]) << 16);
    pos += 3;
    
    // Check if we have enough data
    if (pos + dataLength > data.size()) {
        std::cerr << "Incomplete TZX Direct Recording Block data" << std::endl;
        return data.size();
    }
    
    // This block contains raw samples, not structured data like TAP blocks
    // We could convert it to a TapBlock, but it's complex and may not be necessary
    std::cout << "Skipping TZX Direct Recording Block: " << dataLength << " samples" << std::endl;
    
    // Move position past the data
    pos += dataLength;
    
    return pos;
}

// Parse TZX Pause Block (ID 20)
size_t Tape::parseTzxPauseBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Pause Block" << std::endl;
        return data.size();
    }
    
    // Read pause duration (2 bytes)
    uint16_t pauseDuration = static_cast<uint16_t>(data[pos]) | 
                            (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Pause Block: " << pauseDuration << " ms" << std::endl;
    
    return pos;
}

// Parse TZX Group Start Block (ID 21)
size_t Tape::parseTzxGroupStartBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 1 > data.size()) {
        std::cerr << "Incomplete TZX Group Start Block" << std::endl;
        return data.size();
    }
    
    // Read group name length
    uint8_t groupNameLength = data[pos];
    pos += 1;
    
    // Check if we have enough data for the group name
    if (pos + groupNameLength > data.size()) {
        std::cerr << "Incomplete TZX Group Start Block data" << std::endl;
        return data.size();
    }
    
    // Read group name
    std::string groupName(data.begin() + pos, data.begin() + pos + groupNameLength);
    pos += groupNameLength;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Group Start Block: " << groupName << std::endl;
    
    return pos;
}

// Parse TZX Group End Block (ID 22)
size_t Tape::parseTzxGroupEndBlock(const std::vector<uint8_t>& data, size_t pos) {
    // This block has no body, so we just return the current position
    std::cout << "Skipping TZX Group End Block" << std::endl;
    return pos;
}

// Parse TZX Jump Block (ID 23)
size_t Tape::parseTzxJumpBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Jump Block" << std::endl;
        return data.size();
    }
    
    // Read relative jump value (2 bytes, signed)
    int16_t jumpValue = static_cast<int16_t>(data[pos]) | 
                       (static_cast<int16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // This block affects control flow, but we're just parsing sequentially
    std::cout << "Skipping TZX Jump Block: " << jumpValue << std::endl;
    
    return pos;
}

// Parse TZX Loop Start Block (ID 24)
size_t Tape::parseTzxLoopStartBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Loop Start Block" << std::endl;
        return data.size();
    }
    
    // Read number of repetitions (2 bytes)
    uint16_t repetitions = static_cast<uint16_t>(data[pos]) | 
                          (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // This block affects control flow, but we're just parsing sequentially
    std::cout << "Skipping TZX Loop Start Block: " << repetitions << " repetitions" << std::endl;
    
    return pos;
}

// Parse TZX Loop End Block (ID 25)
size_t Tape::parseTzxLoopEndBlock(const std::vector<uint8_t>& data, size_t pos) {
    // This block has no body, so we just return the current position
    std::cout << "Skipping TZX Loop End Block" << std::endl;
    return pos;
}

// Parse TZX Call Sequence Block (ID 26)
size_t Tape::parseTzxCallSequenceBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Call Sequence Block" << std::endl;
        return data.size();
    }
    
    // Read number of calls (2 bytes)
    uint16_t callCount = static_cast<uint16_t>(data[pos]) | 
                        (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Check if we have enough data for all calls
    if (pos + (2 * callCount) > data.size()) {
        std::cerr << "Incomplete TZX Call Sequence Block data" << std::endl;
        return data.size();
    }
    
    // Skip all call offsets (2 bytes each)
    pos += 2 * callCount;
    
    // This block affects control flow, but we're just parsing sequentially
    std::cout << "Skipping TZX Call Sequence Block: " << callCount << " calls" << std::endl;
    
    return pos;
}

// Parse TZX Return Sequence Block (ID 27)
size_t Tape::parseTzxReturnSequenceBlock(const std::vector<uint8_t>& data, size_t pos) {
    // This block has no body, so we just return the current position
    std::cout << "Skipping TZX Return Sequence Block" << std::endl;
    return pos;
}

// Parse TZX Select Block (ID 28)
size_t Tape::parseTzxSelectBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Select Block" << std::endl;
        return data.size();
    }
    
    // Read block length (2 bytes)
    uint16_t blockLength = static_cast<uint16_t>(data[pos]) | 
                          (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Check if we have enough data for the rest of the block
    if (pos + blockLength > data.size()) {
        std::cerr << "Incomplete TZX Select Block data" << std::endl;
        return data.size();
    }
    
    // Move position past the block data
    pos += blockLength;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Select Block" << std::endl;
    
    return pos;
}

// Parse TZX Stop the Tape if in 48K Mode Block (ID 2A)
size_t Tape::parseTzxStop48KBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 4 > data.size()) {
        std::cerr << "Incomplete TZX Stop the Tape if in 48K Mode Block" << std::endl;
        return data.size();
    }
    
    // Read block length (4 bytes) - should be 0
    uint32_t blockLength = static_cast<uint32_t>(data[pos]) |
                          (static_cast<uint32_t>(data[pos + 1]) << 8) |
                          (static_cast<uint32_t>(data[pos + 2]) << 16) |
                          (static_cast<uint32_t>(data[pos + 3]) << 24);
    pos += 4;
    
    // This block has no additional data, so we just return the current position
    std::cout << "Skipping TZX Stop the Tape if in 48K Mode Block" << std::endl;
    
    return pos;
}

// Parse TZX Set Signal Level Block (ID 2B)
size_t Tape::parseTzxSetLevelBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 5 > data.size()) {
        std::cerr << "Incomplete TZX Set Signal Level Block" << std::endl;
        return data.size();
    }
    
    // Read block length (4 bytes) - should be 1
    uint32_t blockLength = static_cast<uint32_t>(data[pos]) |
                          (static_cast<uint32_t>(data[pos + 1]) << 8) |
                          (static_cast<uint32_t>(data[pos + 2]) << 16) |
                          (static_cast<uint32_t>(data[pos + 3]) << 24);
    pos += 4;
    
    // Read signal level (1 byte)
    uint8_t signalLevel = data[pos];
    pos += 1;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Set Signal Level Block: " << (int)signalLevel << std::endl;
    
    return pos;
}

// Parse TZX Text Description Block (ID 30)
size_t Tape::parseTzxTextDescriptionBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 1 > data.size()) {
        std::cerr << "Incomplete TZX Text Description Block" << std::endl;
        return data.size();
    }
    
    // Read text length
    uint8_t textLength = data[pos];
    pos += 1;
    
    // Check if we have enough data for the text
    if (pos + textLength > data.size()) {
        std::cerr << "Incomplete TZX Text Description Block data" << std::endl;
        return data.size();
    }
    
    // Read text
    std::string text(data.begin() + pos, data.begin() + pos + textLength);
    pos += textLength;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Text Description Block: " << text << std::endl;
    
    return pos;
}

// Parse TZX Message Block (ID 31)
size_t Tape::parseTzxMessageBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Message Block" << std::endl;
        return data.size();
    }
    
    // Read time to display (1 byte)
    uint8_t timeToDisplay = data[pos];
    pos += 1;
    
    // Read message length (1 byte)
    uint8_t messageLength = data[pos];
    pos += 1;
    
    // Check if we have enough data for the message
    if (pos + messageLength > data.size()) {
        std::cerr << "Incomplete TZX Message Block data" << std::endl;
        return data.size();
    }
    
    // Read message
    std::string message(data.begin() + pos, data.begin() + pos + messageLength);
    pos += messageLength;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Message Block: " << message << std::endl;
    
    return pos;
}

// Parse TZX Archive Info Block (ID 32)
size_t Tape::parseTzxArchiveInfoBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 2 > data.size()) {
        std::cerr << "Incomplete TZX Archive Info Block" << std::endl;
        return data.size();
    }
    
    // Read block length (2 bytes)
    uint16_t blockLength = static_cast<uint16_t>(data[pos]) | 
                          (static_cast<uint16_t>(data[pos + 1]) << 8);
    pos += 2;
    
    // Check if we have enough data for the rest of the block
    if (pos + blockLength > data.size()) {
        std::cerr << "Incomplete TZX Archive Info Block data" << std::endl;
        return data.size();
    }
    
    // Read number of strings
    if (pos + 1 > data.size()) {
        std::cerr << "Incomplete TZX Archive Info Block string count" << std::endl;
        return data.size();
    }
    
    uint8_t stringCount = data[pos];
    pos += 1;
    
    // Skip all strings
    for (uint8_t i = 0; i < stringCount; i++) {
        if (pos + 2 > data.size()) {
            std::cerr << "Incomplete TZX Archive Info Block string" << std::endl;
            return data.size();
        }
        
        // Read text ID (1 byte)
        uint8_t textId = data[pos];
        pos += 1;
        
        // Read text length (1 byte)
        uint8_t textLength = data[pos];
        pos += 1;
        
        // Check if we have enough data for the text
        if (pos + textLength > data.size()) {
            std::cerr << "Incomplete TZX Archive Info Block text" << std::endl;
            return data.size();
        }
        
        // Skip the text
        pos += textLength;
    }
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Archive Info Block: " << (int)stringCount << " strings" << std::endl;
    
    return pos;
}

// Parse TZX Hardware Type Block (ID 33)
size_t Tape::parseTzxHardwareTypeBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 1 > data.size()) {
        std::cerr << "Incomplete TZX Hardware Type Block" << std::endl;
        return data.size();
    }
    
    // Read number of hardware entries
    uint8_t hardwareCount = data[pos];
    pos += 1;
    
    // Check if we have enough data for all hardware entries
    if (pos + (3 * hardwareCount) > data.size()) {
        std::cerr << "Incomplete TZX Hardware Type Block data" << std::endl;
        return data.size();
    }
    
    // Skip all hardware entries (3 bytes each)
    pos += 3 * hardwareCount;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Hardware Type Block: " << (int)hardwareCount << " entries" << std::endl;
    
    return pos;
}

// Parse TZX Custom Info Block (ID 35)
size_t Tape::parseTzxCustomInfoBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 14 > data.size()) {
        std::cerr << "Incomplete TZX Custom Info Block header" << std::endl;
        return data.size();
    }
    
    // Read identification string (10 bytes)
    std::string idString(data.begin() + pos, data.begin() + pos + 10);
    pos += 10;
    
    // Read custom info length (4 bytes)
    uint32_t infoLength = static_cast<uint32_t>(data[pos]) |
                         (static_cast<uint32_t>(data[pos + 1]) << 8) |
                         (static_cast<uint32_t>(data[pos + 2]) << 16) |
                         (static_cast<uint32_t>(data[pos + 3]) << 24);
    pos += 4;
    
    // Check if we have enough data for the custom info
    if (pos + infoLength > data.size()) {
        std::cerr << "Incomplete TZX Custom Info Block data" << std::endl;
        return data.size();
    }
    
    // Move position past the custom info
    pos += infoLength;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Custom Info Block: " << idString << std::endl;
    
    return pos;
}

// Parse TZX Glue Block (ID 5A)
size_t Tape::parseTzxGlueBlock(const std::vector<uint8_t>& data, size_t pos) {
    if (pos + 9 > data.size()) {
        std::cerr << "Incomplete TZX Glue Block" << std::endl;
        return data.size();
    }
    
    // Check the glue block signature
    if (memcmp(data.data() + pos, "XTape!", 6) != 0 || data[pos + 6] != 0x1A) {
        std::cerr << "Invalid TZX Glue Block signature" << std::endl;
        return pos + 9;
    }
    
    // Read major and minor version numbers
    uint8_t majorVersion = data[pos + 7];
    uint8_t minorVersion = data[pos + 8];
    pos += 9;
    
    // This block doesn't contain data for TapBlocks, so we just skip it
    std::cout << "Skipping TZX Glue Block: version " << (int)majorVersion << "." << (int)minorVersion << std::endl;
    
    return pos;
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
