#include "sound.hpp"
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>

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
    if (!audioDevice)
    {
        printf("Error opening audio device: %s\n", SDL_GetError());
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        return false;
    }

    // Bind the audio stream to the device
    if (!SDL_BindAudioStream(audioDevice, audioStream))
    {
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
    if ((port & 0xFF) == 0xFE)
    {
        bool micBit = (value & 0x08) == 0; // MIC is bit 3 (0x08) - active low
        bool earBit = (value & 0x10) != 0; // EAR is bit 4 (0x10) - active high

        // prevent flooding of sound subsytem when border changes rapidly, but no sound output. Like when tape loads
        if (micBit == lastMicBit && earBit == lastEarBit)
            return;

        lastEarBit = earBit;
        lastMicBit = micBit;
        unsigned int duration = ticks - ticksPassed;
        ticksPassed = ticks;
        // printf("%lld %d %d\n", ticks - ticksPassed, earBit, micBit);

        if (duration < 1000000)
        {
            if (earBit)
            { // Volume on speaker or tape raised to up
                //generateAudio(duration, true);
                //SDL_FlushAudioStream(audioStream);
                //printf("%d 1\n",duration);
            }
            else
            {
                //generateAudio(duration, false);
                //SDL_FlushAudioStream(audioStream);
                //printf("%d 0\n",duration);
            }
        }
        else {
            printf("Skipped %d\n", duration);
        }
    }
}

void Sound::generateAudio(long long ticks, bool value)
{
    if (!initialized || !audioStream)
    {
        return;
    }

    // Calculate duration in seconds
    // 1 tick = 1/3500000 seconds
    double duration = static_cast<double>(ticks) / 3500000.0;

    // Calculate number of samples (assuming 44100 Hz sample rate)
    int sampleRate = 44100;
    // Use proper rounding instead of truncation + arbitrary offset
    int numSamples = static_cast<int>(std::round(duration * sampleRate));

    // If no samples to generate, return early
    if (numSamples <= 0)
    {
        return;
    }

    // Create buffer for samples (16-bit stereo)
    // Allocate memory for stereo samples (2 channels)
    std::vector<int16_t> buffer(numSamples * 2);

    // Generate samples based on speaker membrane position
    int16_t sampleValue = value ? 10000 : -10000; // Amplitude for up/down position

    // Fill buffer with samples
    for (int i = 0; i < numSamples; ++i)
    {
        // Stereo - same value for both channels
        buffer[i * 2] = sampleValue;     // Left channel
        buffer[i * 2 + 1] = sampleValue; // Right channel
    }

    // Put audio data into the stream
    if (!SDL_PutAudioStreamData(audioStream, buffer.data(), numSamples * 2 * sizeof(int16_t)))
    {
        printf("Put audio failed: %s\n", SDL_GetError());
    }
}
