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
}

// Helper function to check if a string ends with a specific suffix
bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
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

// Parse TAP file format
void Tape::parseTap(const std::vector<uint8_t>& data)
{
    // Empty implementation - to be filled later
    std::cout << "Parsing TAP file with " << data.size() << " bytes" << std::endl;
}

// Parse TZX file format
void Tape::parseTzx(const std::vector<uint8_t>& data)
{
    // Empty implementation - to be filled later
    std::cout << "Parsing TZX file with " << data.size() << " bytes" << std::endl;
}

// Get next audio input state for ULA
bool Tape::getNextBit()
{
    return false;
}
