#include "sound.hpp"
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sound::Sound() : audioStream(nullptr), audioDevice(0), initialized(false)
{
}

Sound::~Sound()
{
    cleanup();
}

bool Sound::initialize()
{
    // Check if audio subsystem is available
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
    {
        std::cerr << "SDL audio subsystem not initialized, skipping sound initialization" << std::endl;
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
        printf("Error creating audio stream: %s\n", SDL_GetError());
        return false;
    }
    
    // Create and open audio device, then bind the stream to it
    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!audioDevice) {
        printf("Error opening audio device: %s\n", SDL_GetError());
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        return false;
    }
    
    // Bind the audio stream to the device
    if (!SDL_BindAudioStream(audioDevice, audioStream)) {
        printf("Error binding audio stream: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        audioDevice = 0;
        return false;
    }
    
    // Start audio playback
    SDL_ResumeAudioDevice(audioDevice);
    
    initialized = true;
    ticks = 0;
    ticksPassed = 0;
    std::cout << "Sound system initialized successfully" << std::endl;
    return true;
}

void Sound::cleanup()
{
    // // Close the audio device first, which should automatically unbind any streams
    // if (audioDevice) {
    //     SDL_CloseAudioDevice(audioDevice);
    //     audioDevice = 0;
    // }
    
    // // Then destroy the audio stream
    // if (audioStream) {
    //     SDL_DestroyAudioStream(audioStream);
    //     audioStream = nullptr;
    // }
    
    initialized = false;
}

void Sound::writePort(uint16_t port, uint8_t value)
{
    // printf("port %d", port);
    if ((port & 0xFF) == 0xFE)
    {
        bool micBit = (value & 0x08) == 0; // MIC is bit 3 (0x08) - active low
        bool earBit = (value & 0x10) != 0; // EAR is bit 4 (0x10) - active high
        // audioState = earBit || micBit;
        // audioState = earBit;
        // printf("%d %d\n", micBit, earBit);
        if (earBit || micBit)
        { // Volume on specker or tape raised to up
            ticksPassed = ticks;
        }
        else
        { // ok, we need to generate some sound
            //printf("%lld\n", ticks - ticksPassed);
            generateAudio(ticks - ticksPassed, true); // Membrana of speaker set to up
            generateAudio(ticks - ticksPassed, false); // set to down
            //generate1000HzTone(1.0);
            //SDL_FlushAudioStream(audioStream);
        }
    }
}

void Sound::generateTone(int frequency, double duration) {
    if (!initialized || !audioStream) {
        return;
    }

    // Calculate number of samples for the requested duration
    int sampleRate = 44100;
    int numSamples = static_cast<int>(duration * sampleRate);
    
    // If no samples to generate, return early
    if (numSamples <= 0) {
        return;
    }
    
    // Create buffer for samples (16-bit stereo)
    std::vector<int16_t> buffer(numSamples * 2);
    
    // Generate sine wave samples
    const double amplitude = 30000.0; // Amplitude for the tone
    const double phaseIncrement = 2.0 * M_PI * frequency / sampleRate;
    double phase = 0.0;
    
    // Fill buffer with sine wave samples
    for (int i = 0; i < numSamples; ++i) {
        int16_t sampleValue = static_cast<int16_t>(amplitude * sin(phase));
        
        // Stereo - same value for both channels
        buffer[i * 2] = sampleValue;     // Left channel
        buffer[i * 2 + 1] = sampleValue; // Right channel
        
        // Advance phase
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }
    
    // Put audio data into the stream
    if(!SDL_PutAudioStreamData(audioStream, buffer.data(), numSamples * 2 * sizeof(int16_t))) {
        printf("Put audio failed: %s\n", SDL_GetError());
    }
    else {
        printf("Generated %d samples for %dHz tone\n", numSamples, frequency);
    }
}

void Sound::generate1000HzTone(double duration) {
    generateTone(1000, duration);
}

void Sound::generateAudio(long long ticks, bool value)
{
    if (!initialized || !audioStream) {
        return;
    }

    // Calculate duration in seconds
    // 1 tick = 1/3500000 seconds
    double duration = static_cast<double>(ticks) / 3500000.0;
    
    // Calculate number of samples (assuming 44100 Hz sample rate)
    int sampleRate = 44100;
    int numSamples = static_cast<int>(duration * sampleRate);
    
    // If no samples to generate, return early
    if (numSamples <= 0) {
        return;
    }
    
    // Create buffer for samples (16-bit stereo)
    // Allocate memory for stereo samples (2 channels)
    std::vector<int16_t> buffer(numSamples * 2);
    
    // Generate samples based on speaker membrane position
    int16_t sampleValue = value ? 10000 : -10000; // Amplitude for up/down position
    
    // Fill buffer with samples
    for (int i = 0; i < numSamples; ++i) {
        // Stereo - same value for both channels
        buffer[i * 2] = sampleValue;     // Left channel
        buffer[i * 2 + 1] = sampleValue; // Right channel
    }
    
    // Put audio data into the stream
    if(!SDL_PutAudioStreamData(audioStream, buffer.data(), numSamples * 2 * sizeof(int16_t))) {
        printf("Put audio failed: %s\n", SDL_GetError());
    }
    else {
        //printf("--- %d\n", numSamples);
    }
}
