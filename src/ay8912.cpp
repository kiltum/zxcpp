#include "ay8912.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

AY8912::AY8912() : audioStream(nullptr), audioDevice(0), initialized(false), selectedRegister(0), addressLatch(false)
{
    // Initialize registers
    std::memset(registers, 0, sizeof(registers));
    
    // Initialize channels
    for (int i = 0; i < 3; i++) {
        channels[i].period = 0;
        channels[i].volume = 0;
        channels[i].enable = true;
        channels[i].counter = 0;
        channels[i].output = false;
    }
    
    // Initialize noise channel
    noise.period = 0;
    noise.volume = 0;
    noise.enable = true;
    noise.counter = 0;
    noise.shiftRegister = 0x1FFFF; // 17-bit shift register
    noise.output = false;
    
    // Initialize envelope
    envelope.period = 0;
    envelope.shape = 0;
    envelope.counter = 0;
    envelope.level = 0;
    envelope.hold = false;
    envelope.alternate = false;
    envelope.attack = false;
    envelope.output = false;
}

AY8912::~AY8912()
{
    cleanup();
}

bool AY8912::initialize()
{
    // Check if audio subsystem is available
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
    {
        std::cerr << "SDL audio subsystem not initialized, skipping AY-3-8912 initialization" << std::endl;
        return false;
    }

    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);
    desired_spec.format = SDL_AUDIO_S16;
    desired_spec.freq = 44100;
    desired_spec.channels = 2;

    audioStream = SDL_CreateAudioStream(&desired_spec, &desired_spec);
    if (!audioStream)
    {
        printf("Error creating AY-3-8912 audio stream: %s\n", SDL_GetError());
        return false;
    }

    // Create and open audio device, then bind the stream to it
    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!audioDevice)
    {
        printf("Error opening AY-3-8912 audio device: %s\n", SDL_GetError());
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        return false;
    }

    // Bind the audio stream to the device
    if (!SDL_BindAudioStream(audioDevice, audioStream))
    {
        printf("Error binding AY-3-8912 audio stream: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        audioDevice = 0;
        return false;
    }

    // Start audio playback
    SDL_ResumeAudioDevice(audioDevice);

    initialized = true;
    reset();
    std::cout << "AY-3-8912 sound chip initialized successfully" << std::endl;
    return true;
}

void AY8912::cleanup()
{
    initialized = false;
}

void AY8912::reset()
{
    // Reset all registers to 0
    std::memset(registers, 0, sizeof(registers));
    selectedRegister = 0;
    addressLatch = false;
    
    // Reset channels
    for (int i = 0; i < 3; i++) {
        channels[i].period = 0;
        channels[i].volume = 0;
        channels[i].enable = true;
        channels[i].counter = 0;
        channels[i].output = false;
    }
    
    // Reset noise
    noise.period = 0;
    noise.volume = 0;
    noise.enable = true;
    noise.counter = 0;
    noise.shiftRegister = 0x1FFFF;
    noise.output = false;
    
    // Reset envelope
    envelope.period = 0;
    envelope.shape = 0;
    envelope.counter = 0;
    envelope.level = 0;
    envelope.hold = false;
    envelope.alternate = false;
    envelope.attack = false;
    envelope.output = false;
}

void AY8912::writePort(uint16_t port, uint8_t value)
{
    // ZX Spectrum 128 uses port BFFDh for writing to AY-3-8912
    if ((port & 0xFFFD) == 0xBFFD) {
        if (addressLatch) {
            // Write data to selected register
            writeRegister(selectedRegister, value);
            addressLatch = false;
        } else {
            // Latch address
            selectedRegister = value & 0x0F; // Only lower 4 bits are valid
            addressLatch = true;
        }
    }
}

uint8_t AY8912::readPort(uint16_t port)
{
    // ZX Spectrum 128 uses port FFFDh for reading from AY-3-8912
    if ((port & 0xFFFD) == 0xFFFD) {
        if (addressLatch) {
            // Read from selected register
            return readRegister(selectedRegister);
        } else {
            // Return 0 if no address is latched
            return 0;
        }
    }
    return 0;
}

void AY8912::writeRegister(uint8_t reg, uint8_t value)
{
    if (reg > 13) return; // Only registers 0-13 are valid
    
    registers[reg] = value;
    
    switch (reg) {
        case 0: // Channel A period (low 8 bits)
            channels[0].period = (channels[0].period & 0xF00) | value;
            break;
        case 1: // Channel A period (high 4 bits)
            channels[0].period = (channels[0].period & 0xFF) | ((value & 0x0F) << 8);
            break;
        case 2: // Channel B period (low 8 bits)
            channels[1].period = (channels[1].period & 0xF00) | value;
            break;
        case 3: // Channel B period (high 4 bits)
            channels[1].period = (channels[1].period & 0xFF) | ((value & 0x0F) << 8);
            break;
        case 4: // Channel C period (low 8 bits)
            channels[2].period = (channels[2].period & 0xF00) | value;
            break;
        case 5: // Channel C period (high 4 bits)
            channels[2].period = (channels[2].period & 0xFF) | ((value & 0x0F) << 8);
            break;
        case 6: // Noise period
            noise.period = value & 0x1F; // Only lower 5 bits
            break;
        case 7: // Mixer
            // Bit 0: Channel A enable (0 = enable, 1 = disable)
            // Bit 1: Channel B enable (0 = enable, 1 = disable)
            // Bit 2: Channel C enable (0 = enable, 1 = disable)
            // Bit 3: Noise enable A (0 = enable, 1 = disable)
            // Bit 4: Noise enable B (0 = enable, 1 = disable)
            // Bit 5: Noise enable C (0 = enable, 1 = disable)
            channels[0].enable = !(value & 0x01);
            channels[1].enable = !(value & 0x02);
            channels[2].enable = !(value & 0x04);
            // Noise enable bits are handled in updateNoise()
            break;
        case 8: // Channel A volume
            channels[0].volume = value & 0x0F; // Only lower 4 bits
            break;
        case 9: // Channel B volume
            channels[1].volume = value & 0x0F; // Only lower 4 bits
            break;
        case 10: // Channel C volume
            channels[2].volume = value & 0x0F; // Only lower 4 bits
            break;
        case 11: // Envelope period (low 8 bits)
            envelope.period = (envelope.period & 0xF00) | value;
            break;
        case 12: // Envelope period (high 8 bits)
            envelope.period = (envelope.period & 0xFF) | ((value & 0xFF) << 8);
            break;
        case 13: // Envelope shape
            envelope.shape = value & 0x0F; // Only lower 4 bits
            envelope.hold = (value & 0x01) != 0;
            envelope.alternate = (value & 0x02) != 0;
            envelope.attack = (value & 0x04) != 0;
            envelope.level = envelope.attack ? 15 : 0;
            break;
    }
}

uint8_t AY8912::readRegister(uint8_t reg)
{
    if (reg > 13) return 0;
    return registers[reg];
}

void AY8912::updateChannels()
{
    for (int i = 0; i < 3; i++) {
        if (channels[i].period > 0) {
            channels[i].counter++;
            if (channels[i].counter >= channels[i].period) {
                channels[i].counter = 0;
                channels[i].output = !channels[i].output;
            }
        }
    }
}

void AY8912::updateNoise()
{
    if (noise.period > 0) {
        noise.counter++;
        if (noise.counter >= noise.period) {
            noise.counter = 0;
            
            // Generate new noise bit
            uint8_t newBit = ((noise.shiftRegister >> 16) & 1) ^ ((noise.shiftRegister >> 14) & 1);
            noise.shiftRegister = ((noise.shiftRegister << 1) | newBit) & 0x1FFFF;
            
            noise.output = (noise.shiftRegister & 1) != 0;
        }
    }
}

void AY8912::updateEnvelope()
{
    if (envelope.period > 0) {
        envelope.counter++;
        if (envelope.counter >= envelope.period) {
            envelope.counter = 0;
            
            if (envelope.attack) {
                if (envelope.level < 15) {
                    envelope.level++;
                } else if (envelope.hold) {
                    envelope.attack = false;
                }
            } else {
                if (envelope.level > 0) {
                    envelope.level--;
                } else if (envelope.hold) {
                    envelope.attack = true;
                }
            }
        }
    }
}

int16_t AY8912::getOutputLevel()
{
    if (!initialized) return 0;
    
    int16_t output = 0;
    
    // Mix the three channels
    for (int i = 0; i < 3; i++) {
        if (channels[i].enable && channels[i].output) {
            int16_t channelLevel = (channels[i].volume * 1000) / 15; // Scale to reasonable level
            output += channelLevel;
        }
    }
    
    // Add noise if enabled
    if (noise.enable && noise.output) {
        int16_t noiseLevel = (noise.volume * 500) / 15; // Lower level for noise
        output += noiseLevel;
    }
    
    // Clamp to 16-bit range
    output = std::max<int16_t>(-32768, std::min<int16_t>(32767, output));
    
    return output;
}

void AY8912::updateAudio()
{
    if (!initialized || !audioStream) return;
    
    // Update all sound generators
    updateChannels();
    updateNoise();
    updateEnvelope();
    
    // Generate audio samples
    generateSamples(1); // Generate one sample per call
}

void AY8912::generateSamples(int numSamples)
{
    if (!initialized || !audioStream) return;
    
    // Create buffer for samples (16-bit stereo)
    std::vector<int16_t> buffer(numSamples * 2);
    
    for (int i = 0; i < numSamples; i++) {
        int16_t sample = getOutputLevel();
        
        // Stereo - same value for both channels
        buffer[i * 2] = sample;     // Left channel
        buffer[i * 2 + 1] = sample; // Right channel
    }
    
    // Put audio data into the stream
    if (!SDL_PutAudioStreamData(audioStream, buffer.data(), numSamples * 2 * sizeof(int16_t)))
    {
        printf("AY-3-8912 audio put failed: %s\n", SDL_GetError());
    }
}
