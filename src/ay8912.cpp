#include "ay8912.hpp"
#include "chips/ay-3-8910.h"
#include <iostream>
#include <cstring>
#include <SDL3/SDL.h>
#include <thread>
#include <chrono>
#include <vector>

AY8912::AY8912() : 
    selectedRegister(0),
    addressLatch(false),
    audioStream(nullptr),
    audioDevice(0),
    initialized(false),
    audioThreadRunning(false)
{
    // Initialize registers
    std::memset(registers, 0, sizeof(registers));
    
    // Initialize the vgm_decoder AY-3-8910 emulator
    ayChip = new AY38910(CHIP_TYPE_AY8910, 0);
    ayChip->setFrequency(1773400); // ZX Spectrum clock frequency
    ayChip->setSampleFrequency(44100); // Audio sample frequency
    ayChip->reset();
}

AY8912::~AY8912()
{
    cleanup();
    if (ayChip) {
        delete ayChip;
        ayChip = nullptr;
    }
}

bool AY8912::initialize()
{
    // Check if audio subsystem is available
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
    {
        std::cerr << "SDL audio subsystem not initialized, skipping AY8912 sound initialization" << std::endl;
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
        printf("Error creating AY8912 audio stream: %s\n", SDL_GetError());
        return false;
    }

    // Create and open audio device, then bind the stream to it
    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!audioDevice)
    {
        printf("Error opening AY8912 audio device: %s\n", SDL_GetError());
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        return false;
    }

    // Bind the audio stream to the device
    if (!SDL_BindAudioStream(audioDevice, audioStream))
    {
        printf("Error binding AY8912 audio stream: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        audioDevice = 0;
        return false;
    }

    // Start audio playback
    SDL_ResumeAudioDevice(audioDevice);

    // Start audio processing thread
    audioThreadRunning = true;
    audioThread = std::thread(&AY8912::processAudio, this);

    initialized = true;
    std::cout << "AY8912 sound system initialized successfully" << std::endl;
    return true;
}

void AY8912::cleanup()
{
    // Stop audio processing thread first
    if (audioThreadRunning) {
        audioThreadRunning = false;
        if (audioThread.joinable()) {
            audioThread.join();
        }
    }

    // Only clean up SDL audio resources if they were successfully initialized
    // and if SDL is still initialized (not shut down yet)
    if (initialized && (SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
        if (audioDevice) {
            SDL_CloseAudioDevice(audioDevice);
            audioDevice = 0;
        }

        if (audioStream) {
            SDL_DestroyAudioStream(audioStream);
            audioStream = nullptr;
        }
    }

    initialized = false;
}

void AY8912::reset()
{
    // Reset registers
    std::memset(registers, 0, sizeof(registers));
    selectedRegister = 0;
    addressLatch = false;
    
    // Reset the AY-3-8910 chip
    if (ayChip) {
        ayChip->reset();
    }
}

void AY8912::writePort(uint16_t port, uint8_t value)
{
    // Check for address/data select port (0xFFFD) - bits 15-14 must be 11, bits 1-0 must be 01
    if ((port & 0xC001) == 0xC001)
    {
        // Write register number
        selectedRegister = value & 0x0F; // Only lower 4 bits are valid
        addressLatch = true;
    }
    
    // Check for register data port (0xBFFD) - bits 15-14 must be 10, bits 1-0 must be 01
    else if ((port & 0xC001) == 0x8001) 
    {
        // Write data to selected register
        if (addressLatch && selectedRegister <= 13)
        {
            registers[selectedRegister] = value;
            addressLatch = false; // Reset latch after writing data
            
            // Pass the register write to the AY-3-8910 emulator
            if (ayChip) {
                ayChip->write(selectedRegister, value);
            }
        }
    }
}

uint8_t AY8912::readPort(uint16_t port)
{
    // Check for address/data select port (0xFFFD) - bits 15-14 must be 11, bits 1-0 must be 01
    if ((port & 0xC001) == 0xC001)
    {
        if (addressLatch && selectedRegister <= 13)
        {
            // Read from selected register
            return registers[selectedRegister];
        }
        else
        {
            // Return 0 if no address is latched
            return 0;
        }
    }
    return 0;
}

void AY8912::processAudio()
{
    const int bufferSize = 1024; // Process 1024 samples at a time
    std::vector<int16_t> audioBuffer(bufferSize * 2); // Stereo samples
    
    while (audioThreadRunning) {
        if (initialized && audioStream && ayChip) {
            // Generate audio samples using the AY-3-8910 emulator
            for (int i = 0; i < bufferSize; i++) {
                uint32_t sample = ayChip->getSample();
                int16_t left = static_cast<int16_t>((sample >> 16) & 0xFFFF);
                int16_t right = static_cast<int16_t>(sample & 0xFFFF);
                
                audioBuffer[i * 2] = left;     // Left channel
                audioBuffer[i * 2 + 1] = right; // Right channel
            }
            
            // Put audio data into the stream
            SDL_PutAudioStreamData(audioStream, audioBuffer.data(), bufferSize * 2 * sizeof(int16_t));
        }
        
        // Sleep for a short time to avoid consuming too much CPU
        std::this_thread::sleep_for(std::chrono::microseconds(1000)); // 1ms
    }
}
