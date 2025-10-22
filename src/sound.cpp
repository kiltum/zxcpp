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

    // Create audio stream
    // ZX Spectrum audio specifications
    const int SAMPLE_RATE = 44100;  // 44.1 kHz
    const SDL_AudioFormat FORMAT = SDL_AUDIO_F32;  // 32-bit floating point
    const int CHANNELS = 1;  // Mono

    SDL_AudioSpec src_spec, dst_spec;
    SDL_zero(src_spec);
    src_spec.format = FORMAT;
    src_spec.channels = CHANNELS;
    src_spec.freq = SAMPLE_RATE;
    
    SDL_zero(dst_spec);
    dst_spec.format = FORMAT;
    dst_spec.channels = CHANNELS;
    dst_spec.freq = SAMPLE_RATE;

    audioStream = SDL_CreateAudioStream(&src_spec, &dst_spec);
    
    if (!audioStream) {
        std::cerr << "SDL could not create audio stream! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Open audio device
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format = FORMAT;
    spec.channels = CHANNELS;
    spec.freq = SAMPLE_RATE;

    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!audioDevice) {
        std::cerr << "SDL could not open audio device! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Bind the stream to the device
    if (SDL_BindAudioStream(audioDevice, audioStream) != 0) {
        std::cerr << "SDL could not bind audio stream to device! SDL_Error: " << SDL_GetError() << std::endl;
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
