#ifndef EMU_H
#define EMU_H

#include "ula.hpp"
#include "memory.hpp"
#include "port.hpp"
#include "z80.hpp"
#include "kempston.hpp"
#include "tape.hpp"

class Emu
{
public:
    Emu();
    bool mapKeyToSpectrum(int key, bool pressed, bool isRightShift);
private:
    // Emulator hardware components (each represents a real ZX Spectrum chip or subsystem)
    std::unique_ptr<Memory> memory;     // RAM and ROM memory management
    std::unique_ptr<Port> ports;        // Input/output port handling
    std::unique_ptr<Z80> cpu;           // Zilog Z80 CPU emulation
    std::unique_ptr<ULA> ula;           // Sinclair ULA (graphics and keyboard controller)
    std::unique_ptr<Kempston> kempston; // Kempston joystick interface
    std::unique_ptr<Tape> tape;         // Tape loading system
};

#endif // EMU_H
