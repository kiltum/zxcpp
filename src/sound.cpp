#include "sound.hpp"
#include <iostream>
#include <cstring>

Sound::Sound() : audioStream(nullptr), audioDevice(0), initialized(false) {
}

Sound::~Sound() {
    cleanup();
}

bool Sound::initialize() {
    // Check if audio subsystem is available
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
        std::cerr << "SDL audio subsystem not initialized, skipping sound initialization" << std::endl;
        return false;
    }

    // Open audio device first
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format = SDL_AUDIO_F32;  // 32-bit floating point
    spec.channels = 1;            // Mono
    spec.freq = 44100;            // 44.1 kHz

    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!audioDevice) {
        std::cerr << "SDL could not open audio device! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Get the actual device spec
    SDL_AudioSpec deviceSpec;
    int sampleFrames;
    if (SDL_GetAudioDeviceFormat(audioDevice, &deviceSpec, &sampleFrames) != true) {
        std::cerr << "SDL could not get audio device format! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
        return false;
    }

    // Create audio stream that matches the device
    audioStream = SDL_CreateAudioStream(&spec, &deviceSpec);
    
    if (!audioStream) {
        std::cerr << "SDL could not create audio stream! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
        return false;
    }

    // Bind the stream to the device
    if (SDL_BindAudioStream(audioDevice, audioStream) != 0) {
        std::cerr << "SDL could not bind audio stream to device! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
        return false;
    }

    // Set the stream gain (volume)
    SDL_SetAudioStreamGain(audioStream, 1.0f);

    initialized = true;
    std::cout << "Sound system initialized successfully" << std::endl;
    return true;
}

void Sound::run() {
    if (!initialized) {
        return;
    }

    // For now, we're not generating any audio
    // This method will be expanded in the future to generate and queue audio data
    // Example placeholder for future implementation:
    /*
    const size_t BUFFER_SAMPLES = 1024;
    float buffer[BUFFER_SAMPLES];
    
    // Generate audio data (this is where the actual sound generation will happen)
    // generateAudio(buffer, BUFFER_SAMPLES);
    
    // Queue the audio data to the stream
    SDL_PutAudioStreamData(audioStream, buffer, BUFFER_SAMPLES * sizeof(float));
    */
}

void Sound::cleanup() {
    if (audioStream && audioDevice) {
        // Unbind the stream from the device before destroying
        SDL_UnbindAudioStream(audioStream);
    }

    if (audioStream) {
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
    }

    if (audioDevice) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }

    initialized = false;
}
