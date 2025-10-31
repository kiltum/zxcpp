#ifndef AY8912_HPP
#define AY8912_HPP

#include <cstdint>
#include <memory>
#include <SDL3/SDL.h>
#include <thread>
#include <atomic>
#include <mutex>

class AY8912
{
private:
    // AY-3-8912 internal state
    uint8_t registers[16];  // 14 registers (0-13), 14-15 unused
    uint8_t selectedRegister;
    bool addressLatch;
    
    // Audio generation
    SDL_AudioStream *audioStream;
    SDL_AudioDeviceID audioDevice;
    bool initialized;
    
    // Audio thread
    std::thread audioThread;
    std::atomic<bool> audioThreadRunning;
    
    // Synchronization for register access
    mutable std::mutex registerMutex;
    
    // Copy of registers for audio thread
    uint8_t audioRegisters[16];
    
    // Channel state for audio thread
    struct AudioChannel {
        uint16_t period;
        uint8_t volume;
        bool enable;
        uint16_t counter;
        bool output;
    } audioChannels[3];
    
    // Noise channel for audio thread
    struct AudioNoiseChannel {
        uint16_t period;
        uint8_t volume;
        bool enable;
        uint16_t counter;
        uint32_t shiftRegister;
        bool output;
    } audioNoise;
    
    // Envelope generator for audio thread
    struct AudioEnvelope {
        uint16_t period;
        uint8_t shape;
        uint16_t counter;
        uint8_t level;
        bool hold;
        bool alternate;
        bool attack;
        bool output;
    } audioEnvelope;
    
    // Audio generation functions
    void audioThreadFunction();
    int16_t generateSample();
    void updateAudioChannels();
    void updateAudioNoise();
    void updateAudioEnvelope();
    
    // Register handling
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

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
};

#endif // AY8912_HPP
