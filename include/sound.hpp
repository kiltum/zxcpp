#ifndef SOUND_HPP
#define SOUND_HPP

#include <SDL3/SDL.h>
#include <memory>
#include <atomic>

class Sound {
private:
    SDL_AudioStream* audioStream;
    SDL_AudioDeviceID audioDevice;
    bool initialized;
    
public:
    Sound();
    ~Sound();
    
    bool initialize();
    void run();
    void cleanup();
    
    // Future methods for audio generation will go here
    // void generateAudio(float* buffer, size_t samples);
    // void queueAudio(const float* data, size_t samples);
};

#endif // SOUND_HPP
