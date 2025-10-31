#include "ay8912.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <thread>

AY8912::AY8912() : selectedRegister(0), addressLatch(false), audioStream(nullptr), audioDevice(0), initialized(false),
                   audioThreadRunning(false)
{
    // Initialize registers
    std::memset(registers, 0, sizeof(registers));
    std::memset(audioRegisters, 0, sizeof(audioRegisters));

    // Initialize audio channels
    for (int i = 0; i < 3; i++)
    {
        audioChannels[i].period = 0;
        audioChannels[i].volume = 0;
        audioChannels[i].enable = true;
        audioChannels[i].counter = 0;
        audioChannels[i].output = false;
    }

    // Initialize noise channel
    audioNoise.period = 0;
    audioNoise.volume = 0;
    audioNoise.enable = true;
    audioNoise.counter = 0;
    audioNoise.shiftRegister = 0x1FFFF; // 17-bit shift register
    audioNoise.output = false;

    // Initialize envelope
    audioEnvelope.period = 0;
    audioEnvelope.shape = 0;
    audioEnvelope.counter = 0;
    audioEnvelope.level = 0;
    audioEnvelope.hold = false;
    audioEnvelope.alternate = false;
    audioEnvelope.attack = false;
    audioEnvelope.output = false;
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
        std::cerr << "SDL audio subsystem not initialized, skipping AY-3-8912 initialization" << std::endl;
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
        printf("Error creating AY-3-8912 audio stream: %s\n", SDL_GetError());
        return false;
    }

    // Create and open audio device, then bind the stream to it
    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!audioDevice)
    {
        printf("Error opening AY-3-8912 audio device: %s\n", SDL_GetError());
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        return false;
    }

    // Bind the audio stream to the device
    if (!SDL_BindAudioStream(audioDevice, audioStream))
    {
        printf("Error binding AY-3-8912 audio stream: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
        audioDevice = 0;
        return false;
    }

    // Start audio playback
    SDL_ResumeAudioDevice(audioDevice);

    initialized = true;
    reset();
    
    // Start audio thread
    audioThreadRunning = true;
    audioThread = std::thread(&AY8912::audioThreadFunction, this);
    
    std::cout << "AY-3-8912 sound chip initialized successfully" << std::endl;
    return true;
}

void AY8912::cleanup()
{
    // Stop audio thread
    if (audioThreadRunning)
    {
        audioThreadRunning = false;
        if (audioThread.joinable())
        {
            audioThread.join();
        }
    }

    if (audioStream)
    {
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
    }

    if (audioDevice)
    {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }

    initialized = false;
}

void AY8912::reset()
{
    std::lock_guard<std::mutex> lock(registerMutex);
    
    // Reset all registers to 0
    std::memset(registers, 0, sizeof(registers));
    std::memset(audioRegisters, 0, sizeof(audioRegisters));
    selectedRegister = 0;
    addressLatch = false;

    // Reset channels
    for (int i = 0; i < 3; i++)
    {
        audioChannels[i].period = 0;
        audioChannels[i].volume = 0;
        audioChannels[i].enable = true;
        audioChannels[i].counter = 0;
        audioChannels[i].output = false;
    }

    // Reset noise
    audioNoise.period = 0;
    audioNoise.volume = 0;
    audioNoise.enable = true;
    audioNoise.counter = 0;
    audioNoise.shiftRegister = 0x1FFFF;
    audioNoise.output = false;

    // Reset envelope
    audioEnvelope.period = 0;
    audioEnvelope.shape = 0;
    audioEnvelope.counter = 0;
    audioEnvelope.level = 0;
    audioEnvelope.hold = false;
    audioEnvelope.alternate = false;
    audioEnvelope.attack = false;
    audioEnvelope.output = false;
}

void AY8912::writePort(uint16_t port, uint8_t value)
{
    // ZX Spectrum 128 uses:
    // Port FFFDh to write the register number you want to access
    // Port BFFDh to write the data to the register that was just selected
    
    // For compatibility with ZX Spectrum 128, we need to handle port mirroring
    // Ports FFFD and BFFD are mirrored across the address space
    // Check for register selection (FFFD) - mask to check bits 0 and 2
    if ((port & 0x0003) == 0x0001)  // Matches FFFD (xxxx xxxx xxx1 xxxx)
    {
        // Write register number
        selectedRegister = value & 0x0F; // Only lower 4 bits are valid
        addressLatch = true;
    }
    // Check for data write (BFFD) - mask to check bits 0 and 2
    else if ((port & 0x0003) == 0x0003)  // Matches BFFD (xxxx xxxx xxx1 xx1x)
    {
        // Write data to selected register
        if (addressLatch)
        {
            writeRegister(selectedRegister, value);
            addressLatch = false; // Reset latch after writing data
        }
    }
}

uint8_t AY8912::readPort(uint16_t port)
{
    // ZX Spectrum 128 uses port FFFDh for reading from AY-3-8912
    // Check for register read (FFFD) - mask to check bits 0 and 2
    if ((port & 0x0003) == 0x0001)  // Matches FFFD (xxxx xxxx xxx1 xxxx)
    {
        if (addressLatch)
        {
            // Read from selected register
            return readRegister(selectedRegister);
        }
        else
        {
            // Return 0 if no address is latched
            return 0;
        }
    }
    return 0;
}

void AY8912::writeRegister(uint8_t reg, uint8_t value)
{
    if (reg > 13)
        return; // Only registers 0-13 are valid

    {
        std::lock_guard<std::mutex> lock(registerMutex);
        registers[reg] = value;
    }

    // Update audio thread's copy of registers and state
    switch (reg)
    {
    case 0: // Channel A period (low 8 bits)
        audioChannels[0].period = (audioChannels[0].period & 0xF00) | value;
        break;
    case 1: // Channel A period (high 4 bits)
        audioChannels[0].period = (audioChannels[0].period & 0xFF) | ((value & 0x0F) << 8);
        break;
    case 2: // Channel B period (low 8 bits)
        audioChannels[1].period = (audioChannels[1].period & 0xF00) | value;
        break;
    case 3: // Channel B period (high 4 bits)
        audioChannels[1].period = (audioChannels[1].period & 0xFF) | ((value & 0x0F) << 8);
        break;
    case 4: // Channel C period (low 8 bits)
        audioChannels[2].period = (audioChannels[2].period & 0xF00) | value;
        break;
    case 5: // Channel C period (high 4 bits)
        audioChannels[2].period = (audioChannels[2].period & 0xFF) | ((value & 0x0F) << 8);
        break;
    case 6:                          // Noise period
        audioNoise.period = value & 0x1F; // Only lower 5 bits
        break;
    case 7: // Mixer
        // Bit 0: Channel A enable (0 = enable, 1 = disable)
        // Bit 1: Channel B enable (0 = enable, 1 = disable)
        // Bit 2: Channel C enable (0 = enable, 1 = disable)
        // Bit 3: Noise enable A (0 = enable, 1 = disable)
        // Bit 4: Noise enable B (0 = enable, 1 = disable)
        // Bit 5: Noise enable C (0 = enable, 1 = disable)
        audioChannels[0].enable = !(value & 0x01);
        audioChannels[1].enable = !(value & 0x02);
        audioChannels[2].enable = !(value & 0x04);
        // Noise enable bits are handled in updateNoise()
        break;
    case 8:                                // Channel A volume
        audioChannels[0].volume = value & 0x0F; // Only lower 4 bits
        break;
    case 9:                                // Channel B volume
        audioChannels[1].volume = value & 0x0F; // Only lower 4 bits
        break;
    case 10:                               // Channel C volume
        audioChannels[2].volume = value & 0x0F; // Only lower 4 bits
        break;
    case 11: // Envelope period (low 8 bits)
        audioEnvelope.period = (audioEnvelope.period & 0xF00) | value;
        break;
    case 12: // Envelope period (high 8 bits)
        audioEnvelope.period = (audioEnvelope.period & 0xFF) | ((value & 0xFF) << 8);
        break;
    case 13:                           // Envelope shape
        audioEnvelope.shape = value & 0x0F; // Only lower 4 bits
        // Initialize envelope state based on shape
        switch (audioEnvelope.shape)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            // For attack shapes, start at 0
            if (audioEnvelope.shape == 4 || audioEnvelope.shape == 5 || audioEnvelope.shape == 6 ||
                audioEnvelope.shape == 7 || audioEnvelope.shape == 12 || audioEnvelope.shape == 13 ||
                audioEnvelope.shape == 14 || audioEnvelope.shape == 15)
            {
                audioEnvelope.level = 0;
                audioEnvelope.attack = true;
            }
            else
            {
                // For decay shapes, start at 15
                audioEnvelope.level = 15;
                audioEnvelope.attack = false;
            }
            break;
        }
        break;
    }
}

uint8_t AY8912::readRegister(uint8_t reg)
{
    if (reg > 13)
        return 0;
    
    std::lock_guard<std::mutex> lock(registerMutex);
    return registers[reg];
}

void AY8912::audioThreadFunction()
{
    while (audioThreadRunning)
    {
        // Generate one sample
        int16_t sample = generateSample();
        
        // Create stereo sample
        int16_t stereoSample[2] = {sample, sample};
        
        // Put audio data into the stream
        if (audioStream && SDL_PutAudioStreamData(audioStream, stereoSample, 2 * sizeof(int16_t)))
        {
            // Successfully added data to stream
        }
        
        // Sleep to maintain 44.1kHz sample rate
        // Sleep for approximately 1/44100 seconds = ~22.67 microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(22));
    }
}

int16_t AY8912::generateSample()
{
    // Update all sound generators
    updateAudioChannels();
    updateAudioNoise();
    updateAudioEnvelope();

    int16_t output = 0;

    // Mix the three channels
    for (int i = 0; i < 3; i++)
    {
        if (audioChannels[i].enable && audioChannels[i].output)
        {
            // Check if envelope is enabled for this channel (bit 4 set in volume register)
            bool useEnvelope = (audioRegisters[8 + i] & 0x10) != 0;
            int volume = useEnvelope ? audioEnvelope.level : audioChannels[i].volume;
            int16_t channelLevel = (volume * 1000) / 15; // Scale to reasonable level
            output += channelLevel;
        }
    }

    // Add noise if enabled
    if (audioNoise.enable && audioNoise.output)
    {
        // Check if envelope is enabled for noise (bit 4 set in volume register)
        bool useEnvelope = (audioRegisters[8 + 2] & 0x10) != 0; // Use channel C volume register for noise
        int volume = useEnvelope ? audioEnvelope.level : audioNoise.volume;
        int16_t noiseLevel = (volume * 500) / 15; // Lower level for noise
        output += noiseLevel;
    }

    // Clamp to 16-bit range
    output = std::max<int16_t>(-32768, std::min<int16_t>(32767, output));

    return output;
}

void AY8912::updateAudioChannels()
{
    for (int i = 0; i < 3; i++)
    {
        if (audioChannels[i].period > 0)
        {
            audioChannels[i].counter++;
            if (audioChannels[i].counter >= audioChannels[i].period)
            {
                audioChannels[i].counter = 0;
                audioChannels[i].output = !audioChannels[i].output;
            }
        }
    }
}

void AY8912::updateAudioNoise()
{
    if (audioNoise.period > 0)
    {
        audioNoise.counter++;
        if (audioNoise.counter >= audioNoise.period)
        {
            audioNoise.counter = 0;

            // Generate new noise bit
            uint8_t newBit = ((audioNoise.shiftRegister >> 16) & 1) ^ ((audioNoise.shiftRegister >> 14) & 1);
            audioNoise.shiftRegister = ((audioNoise.shiftRegister << 1) | newBit) & 0x1FFFF;

            audioNoise.output = (audioNoise.shiftRegister & 1) != 0;
        }
    }
}

void AY8912::updateAudioEnvelope()
{
    if (audioEnvelope.period > 0)
    {
        audioEnvelope.counter++;
        if (audioEnvelope.counter >= audioEnvelope.period)
        {
            audioEnvelope.counter = 0;

            // Implement all 16 envelope shapes according to AY-3-8912 documentation
            switch (audioEnvelope.shape)
            {
            case 0:  // \__________
            case 1:  // \__________
            case 2:  // \__________
            case 3:  // \__________
            case 9:  // \__________
            case 11: // \__________
                // Single decay then off
                if (audioEnvelope.level > 0)
                {
                    audioEnvelope.level--;
                }
                break;

            case 4:  // /|_________
            case 5:  // /|_________
            case 6:  // /|_________
            case 7:  // /|_________
            case 13: // /_________
            case 15: // /_________
                // Single attack then off
                if (audioEnvelope.level < 15)
                {
                    audioEnvelope.level++;
                }
                break;

            case 8: /* \|\|\|\|\|\ */
                // Repeated decay
                if (audioEnvelope.level > 0)
                {
                    audioEnvelope.level--;
                }
                else
                {
                    audioEnvelope.level = 15; // Restart at maximum
                }
                break;

            case 10: /* \/\/\/\/\/\ */
                // Repeated decay-attack
                if (audioEnvelope.attack)
                {
                    if (audioEnvelope.level < 15)
                    {
                        audioEnvelope.level++;
                    }
                    else
                    {
                        audioEnvelope.attack = false; // Switch to decay
                    }
                }
                else
                {
                    if (audioEnvelope.level > 0)
                    {
                        audioEnvelope.level--;
                    }
                    else
                    {
                        audioEnvelope.attack = true; // Switch to attack
                    }
                }
                break;

            case 12: /* /|/|/|/|/|/ */
                // Repeated attack
                if (audioEnvelope.level < 15)
                {
                    audioEnvelope.level++;
                }
                else
                {
                    audioEnvelope.level = 0; // Restart at minimum
                }
                break;

            case 14: /* /\/\/\/\/\/ */
                // Repeated attack-decay
                if (audioEnvelope.attack)
                {
                    if (audioEnvelope.level < 15)
                    {
                        audioEnvelope.level++;
                    }
                    else
                    {
                        audioEnvelope.attack = false; // Switch to decay
                    }
                }
                else
                {
                    if (audioEnvelope.level > 0)
                    {
                        audioEnvelope.level--;
                    }
                    else
                    {
                        audioEnvelope.attack = true; // Switch to attack
                    }
                }
                break;
            }
        }
    }
}
