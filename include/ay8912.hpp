#ifndef AY8912_HPP
#define AY8912_HPP

#include <cstdint>
#include <SDL3/SDL.h>
#include <thread>
#include <atomic>

// Forward declaration of AY38910 class
class AY38910;

class AY8912
{
private:
    // AY-3-8912 internal state
    uint8_t registers[16];  // 14 registers (0-13), 14-15 unused
    uint8_t selectedRegister;
    bool addressLatch;
    
    // Audio streaming
    SDL_AudioStream* audioStream;
    SDL_AudioDeviceID audioDevice;
    bool initialized;
    
    // Audio processing thread
    std::thread audioThread;
    std::atomic<bool> audioThreadRunning;
    
    // AY-3-8910 emulator instance
    AY38910* ayChip;

public:
    AY8912();
    ~AY8912();
    
    bool initialize();
    void cleanup();
    
    // Port handlers for ZX Spectrum 128
    void writePort(uint16_t port, uint8_t value);
    uint8_t readPort(uint16_t port);
    
    // Reset the chip
    void reset();
    
    // Audio processing
    void processAudio();
    void processAudioTone();
};

#endif // AY8912_HPP
