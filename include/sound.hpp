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
    void writePort(uint16_t port, uint8_t value);
};

#endif // SOUND_HPP
