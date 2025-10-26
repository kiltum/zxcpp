#include "../include/tape.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <fnmatch.h>
#include <algorithm>

class TapeTester {
private:
    std::vector<std::string> testFiles;

public:
    // Find all files matching the pattern in testdata directory
    bool findTestFiles() {
        DIR* dir = opendir("testdata");
        if (!dir) {
            std::cerr << "Error: Could not open testdata directory" << std::endl;
            return false;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Check if filename starts with "ABC" and has a known extension
            std::string filename(entry->d_name);
            if (filename.substr(0, 3) == "ABC" && 
                (filename.length() >= 4 && filename.substr(filename.length()-4) == ".TAP") ||
                (filename.length() >= 4 && filename.substr(filename.length()-4) == ".tap") ||
                (filename.length() >= 4 && filename.substr(filename.length()-4) == ".tzx") ||
                (filename.length() >= 8 && filename.substr(filename.length()-8) == ".tap.zip") ||
                (filename.length() >= 8 && filename.substr(filename.length()-8) == ".tzx.zip")) {
                testFiles.push_back("testdata/" + filename);
            }
        }

        closedir(dir);
        
        // Sort files for consistent ordering
        std::sort(testFiles.begin(), testFiles.end());
        
        if (testFiles.empty()) {
            std::cerr << "Warning: No ABC* test files found in testdata directory" << std::endl;
            return false;
        }
        
        std::cout << "Found " << testFiles.size() << " test files:" << std::endl;
        for (const auto& file : testFiles) {
            std::cout << "  " << file << std::endl;
        }
        
        return true;
    }

    // Test loading a single file
    bool testLoadFile(const std::string& filename) {
        std::cout << "Testing file: " << filename << std::endl;
        
        Tape tape;
        bool result = tape.loadFile(filename);
        
        if (result) {
            std::cout << "  SUCCESS: File loaded successfully" << std::endl;
        } else {
            std::cout << "  FAILED: Could not load file" << std::endl;
        }
        
        return result;
    }

    // Run all tests
    bool runAllTests() {
        if (testFiles.empty()) {
            std::cerr << "No test files to run" << std::endl;
            return false;
        }

        std::cout << "\nRunning " << testFiles.size() << " tape tests..." << std::endl;

        int passed = 0;
        int failed = 0;

        for (const auto& file : testFiles) {
            if (testLoadFile(file)) {
                passed++;
            } else {
                failed++;
            }
        }

        std::cout << "\nTape tests completed: " << passed << " passed, " << failed << " failed" << std::endl;
        return (failed == 0);
    }
};

int main() {
    std::cout << "Tape Loading Test" << std::endl;
    std::cout << "=================" << std::endl;

    TapeTester tester;

    // Find test files
    if (!tester.findTestFiles()) {
        return 1;
    }

    // Run all tests
    bool success = tester.runAllTests();

    return success ? 0 : 1;
}
