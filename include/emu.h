#ifndef EMU_H
#define EMU_H

#include "ula.hpp"
#include "memory.hpp"
#include "port.hpp"
#include "z80.hpp"
#include "kempston.hpp"
#include "tape.hpp"
#include "sound.hpp"
#include "ay8912.hpp"
#include <QtCore/qobject.h>

#define ZX_SPECTRUM_48 0
#define ZX_SPECTRUM_128 1

class Emu : public QObject
{
    Q_OBJECT
public:
    Emu();
    bool mapKeyToSpectrum(int key, bool pressed, bool isRightShift);
    uint32_t cpuSpeed; // Z80 frequency
    uint busDelimeter; // used in turbomodes, how many ticks z80 can do before others see one.
    void Reset(); // reset all emulator
    void setMemoryType(uint type);
    void setULAType(uint type);
    uint32_t *getScreenBuffer();
    void Run(void);
signals:
    void updateScreen(void);
private:
    // Emulator hardware components (each represents a real ZX Spectrum chip or subsystem)
    std::unique_ptr<Memory> memory;     // RAM and ROM memory management
    std::unique_ptr<Port> ports;        // Input/output port handling
    std::unique_ptr<Z80> cpu;           // Zilog Z80 CPU emulation
    std::unique_ptr<ULA> ula;           // Sinclair ULA (graphics and keyboard controller)
    std::unique_ptr<Kempston> kempston; // Kempston joystick interface
    std::unique_ptr<Tape> tape;         // Tape loading system
    std::unique_ptr<Sound> sound;       // Beeper sound system
    std::unique_ptr<AY8912> ay8912;     // AY-3-8912 sound chip (for better sound)

    uint memoryType;
    uint ulaType;

    // Thread management for running the emulation in the background
    std::thread emulationThread;     // Thread that runs the Z80 CPU emulation
    std::atomic<bool> threadRunning; // Atomic flag to safely control thread execution
    std::chrono::high_resolution_clock::time_point lastScreenUpdate; // Last time screen was updated
    const std::chrono::milliseconds minScreenUpdateInterval{100};    // Minimum time between screen updates (10 FPS)

    uint CHECK_INTERVAL;   // How often to check timing (in CPU cycles)
};

#endif // EMU_H
