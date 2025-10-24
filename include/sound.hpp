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
    long long ticksPassed;
    
public:
    Sound();
    ~Sound();
    
    bool initialize();
    void cleanup();
    void writePort(uint16_t port, uint8_t value);
    void generateAudio(long long ticks, bool value);
    void generateTone(int frequency, double duration);
    void generate1000HzTone(double duration = 1.0);
    long long ticks;
};

#endif // SOUND_HPP
