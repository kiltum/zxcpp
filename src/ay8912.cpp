#include "ay8912.hpp"
#include <iostream>
#include <cstring>
#include <SDL3/SDL.h>

AY8912::AY8912() : selectedRegister(0), addressLatch(false), audioStream(nullptr), audioDevice(0), initialized(false)
{
    // Initialize registers
    std::memset(registers, 0, sizeof(registers));
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

    initialized = true;
    std::cout << "AY8912 sound system initialized successfully" << std::endl;
    return true;
}

void AY8912::cleanup()
{
    // if (audioDevice) {
    //     SDL_CloseAudioDevice(audioDevice);
    //     audioDevice = 0;
    // }

    // if (audioStream) {
    //     SDL_DestroyAudioStream(audioStream);
    //     audioStream = nullptr;
    // }

    initialized = false;
}

void AY8912::reset()
{
    // Reset registers
    std::memset(registers, 0, sizeof(registers));
    selectedRegister = 0;
    addressLatch = false;
}

void AY8912::writePort(uint16_t port, uint8_t value)
{
    if (port == 0xFFFD)
    {
        printf("AD %x %x\n",port,value); // Show what to write to port for debug
        // Write register number
        selectedRegister = value & 0x0F; // Only lower 4 bits are valid
        addressLatch = true;
    }
    
    else if (port == 0xBFFD) 
    {
        printf("AR %x %x\n",port,value); // Show what to write to port for debug
        // Write data to selected register
        if (addressLatch && selectedRegister <= 13)
        {
            registers[selectedRegister] = value;
            addressLatch = false; // Reset latch after writing data
        }
    }
}

uint8_t AY8912::readPort(uint16_t port)
{
    if (port == 0xFFFD)
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
