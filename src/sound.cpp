#include "sound.hpp"
#include <iostream>
#include <cstring>
#include <vector>

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

    SDL_AudioSpec desired_src_spec;
    SDL_zero(desired_src_spec);
    desired_src_spec.format = SDL_AUDIO_S16;
    desired_src_spec.freq = 44100;
    desired_src_spec.channels = 2;

    SDL_AudioSpec desired_dst_spec;
    SDL_zero(desired_dst_spec);
    desired_dst_spec.format = SDL_AUDIO_S16;
    desired_dst_spec.freq = 48000;
    desired_dst_spec.channels = 2;

    audioStream = SDL_CreateAudioStream(&desired_src_spec, &desired_dst_spec);
    if (!audioStream)
    {
        printf("Error creating audio stream: %s\n", SDL_GetError());
        return false;
    }
    // For now, just initialize without creating streams
    // We'll add proper audio stream implementation later
    initialized = true;
    ticks = 0;
    ticksPassed = 0;
    std::cout << "Sound system initialized successfully" << std::endl;
    return true;
}

void Sound::cleanup()
{
    // For now, just reset the flags since we're not creating actual streams
    //SDL_DestroyAudioStream(audioStream);
    audioStream = nullptr;
    audioDevice = 0;
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
            printf("%lld\n", ticks - ticksPassed);
            generateAudio(ticks - ticksPassed, true); // Membrana of speaker set to up
            generateAudio(ticks - ticksPassed, false); // set to down
            //SDL_FlushAudioStream(audioStream);
        }
    }
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
        printf("--- %d\n", numSamples);
    }
}
