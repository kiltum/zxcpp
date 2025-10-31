#ifndef AY8912_HPP
#define AY8912_HPP

#include <cstdint>
#include <memory>
#include <SDL3/SDL.h>

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
    
    // Audio resampling - accumulator for CPU cycles
    long long cpuCycleAccumulator;
    
    // Channel state
    struct Channel {
        uint16_t period;        // 12-bit period counter
        uint8_t volume;         // 4-bit volume (0-15)
        bool enable;            // Channel enable/disable
        uint16_t counter;       // Current counter value
        bool output;            // Current output level
    } channels[3];
    
    // Noise channel
    struct NoiseChannel {
        uint16_t period;        // 5-bit period counter
        uint8_t volume;         // 4-bit volume (0-15)
        bool enable;            // Noise enable/disable
        uint16_t counter;       // Current counter value
        uint32_t shiftRegister; // 17-bit shift register
        bool output;            // Current output level
    } noise;
    
    // Envelope generator
    struct Envelope {
        uint16_t period;        // 16-bit period counter
        uint8_t shape;          // Envelope shape (4 bits)
        uint16_t counter;       // Current counter value
        uint8_t level;          // Current envelope level (0-15)
        bool hold;              // Hold state
        bool alternate;         // Alternate state
        bool attack;            // Attack phase
        bool output;            // Current output level
    } envelope;
    
    // Mixing and audio generation
    void generateSamples(int numSamples);
    void updateChannels();
    void updateNoise();
    void updateEnvelope();
    
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
    
    // Get current audio output level for mixing
    int16_t getOutputLevel();
    
    // Update audio for each CPU tick
    void updateAudio(bool is48kMode = true);
};

#endif // AY8912_HPP
