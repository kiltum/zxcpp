#include <QKeyEvent>

#include "emu.h"

Emu::Emu() {
    memory = std::make_unique<Memory>();
    ports = std::make_unique<Port>();
    cpu = std::make_unique<Z80>(memory.get(), ports.get());
    tape = std::make_unique<Tape>();
    ula = std::make_unique<ULA>(memory.get(), tape.get());
    kempston = std::make_unique<Kempston>();

    ports->RegisterReadHandler(0xFE, [this](uint16_t port) -> uint8_t
                               { return ula->readPort(port); });
    ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value)
                                { ula->writePort(port, value); });


    ports->RegisterReadHandler(0x1F, [this](uint16_t port) -> uint8_t
                               { return kempston->readPort(port); });

    ports->RegisterWriteHandler(0xFD, [this](uint16_t port, uint8_t value)
                                { return memory->writePort(port, value); });

    cpu->isNMOS = false;
}

bool Emu::mapKeyToSpectrum(int key, bool pressed, bool isRightShift)
{
    // This function maps PC keyboard keys to the ZX Spectrum's keyboard matrix
    // The ZX Spectrum keyboard is organized in a grid of rows and columns
    // Each key press connects a specific row and column

    // Row and column mappings for each key:
    // Row 0: CAPS SHIFT, Z, X, C, V
    // Row 1: A, S, D, F, G
    // Row 2: Q, W, E, R, T
    // Row 3: 1, 2, 3, 4, 5
    // Row 4: 0, 9, 8, 7, 6
    // Row 5: P, O, I, U, Y
    // Row 6: ENTER, L, K, J, H
    // Row 7: SPACE, SYM SHIFT, M, N, B

    bool handled = true;

    switch (key)
    {
    // Row 0: CAPS SHIFT, Z, X, C, V
    case Qt::Key_Shift:
        if(isRightShift) {
            if (pressed)
                ula->setKeyDown(7, 1);
            else
                ula->setKeyUp(7, 1); // SYM SHIFT
        } else {
        if (pressed)
            ula->setKeyDown(0, 0);
        else
            ula->setKeyUp(0, 0); // CAPS SHIFT
        }
        break;

    case Qt::Key_Z:
        if (pressed)
            ula->setKeyDown(0, 1);
        else
            ula->setKeyUp(0, 1); // Z
        break;
    case Qt::Key_X:
        if (pressed)
            ula->setKeyDown(0, 2);
        else
            ula->setKeyUp(0, 2); // X
        break;
    case Qt::Key_C:
        if (pressed)
            ula->setKeyDown(0, 3);
        else
            ula->setKeyUp(0, 3); // C
        break;
    case Qt::Key_V:
        if (pressed)
            ula->setKeyDown(0, 4);
        else
            ula->setKeyUp(0, 4); // V
        break;

    // Row 1: A, S, D, F, G
    case Qt::Key_A:
        if (pressed)
            ula->setKeyDown(1, 0);
        else
            ula->setKeyUp(1, 0); // A
        break;
    case Qt::Key_S:
        if (pressed)
            ula->setKeyDown(1, 1);
        else
            ula->setKeyUp(1, 1); // S
        break;
    case Qt::Key_D:
        if (pressed)
            ula->setKeyDown(1, 2);
        else
            ula->setKeyUp(1, 2); // D
        break;
    case Qt::Key_F:
        if (pressed)
            ula->setKeyDown(1, 3);
        else
            ula->setKeyUp(1, 3); // F
        break;
    case Qt::Key_G:
        if (pressed)
            ula->setKeyDown(1, 4);
        else
            ula->setKeyUp(1, 4); // G
        break;

    // Row 2: Q, W, E, R, T
    case Qt::Key_Q:
        if (pressed)
            ula->setKeyDown(2, 0);
        else
            ula->setKeyUp(2, 0); // Q
        break;
    case Qt::Key_W:
        if (pressed)
            ula->setKeyDown(2, 1);
        else
            ula->setKeyUp(2, 1); // W
        break;
    case Qt::Key_E:
        if (pressed)
            ula->setKeyDown(2, 2);
        else
            ula->setKeyUp(2, 2); // E
        break;
    case Qt::Key_R:
        if (pressed)
            ula->setKeyDown(2, 3);
        else
            ula->setKeyUp(2, 3); // R
        break;
    case Qt::Key_T:
        if (pressed)
            ula->setKeyDown(2, 4);
        else
            ula->setKeyUp(2, 4); // T
        break;

    // Row 3: 1, 2, 3, 4, 5
    case Qt::Key_1:
        if (pressed)
            ula->setKeyDown(3, 0);
        else
            ula->setKeyUp(3, 0); // 1
        break;
    case Qt::Key_2:
        if (pressed)
            ula->setKeyDown(3, 1);
        else
            ula->setKeyUp(3, 1); // 2
        break;
    case Qt::Key_3:
        if (pressed)
            ula->setKeyDown(3, 2);
        else
            ula->setKeyUp(3, 2); // 3
        break;
    case Qt::Key_4:
        if (pressed)
            ula->setKeyDown(3, 3);
        else
            ula->setKeyUp(3, 3); // 4
        break;
    case Qt::Key_5:
        if (pressed)
            ula->setKeyDown(3, 4);
        else
            ula->setKeyUp(3, 4); // 5
        break;

    // Row 4: 0, 9, 8, 7, 6
    case Qt::Key_0:
        if (pressed)
            ula->setKeyDown(4, 0);
        else
            ula->setKeyUp(4, 0); // 0
        break;
    case Qt::Key_9:
        if (pressed)
            ula->setKeyDown(4, 1);
        else
            ula->setKeyUp(4, 1); // 9
        break;
    case Qt::Key_8:
        if (pressed)
            ula->setKeyDown(4, 2);
        else
            ula->setKeyUp(4, 2); // 8
        break;
    case Qt::Key_7:
        if (pressed)
            ula->setKeyDown(4, 3);
        else
            ula->setKeyUp(4, 3); // 7
        break;
    case Qt::Key_6:
        if (pressed)
            ula->setKeyDown(4, 4);
        else
            ula->setKeyUp(4, 4); // 6
        break;

    // Row 5: P, O, I, U, Y
    case Qt::Key_P:
        if (pressed)
            ula->setKeyDown(5, 0);
        else
            ula->setKeyUp(5, 0); // P
        break;
    case Qt::Key_O:
        if (pressed)
            ula->setKeyDown(5, 1);
        else
            ula->setKeyUp(5, 1); // O
        break;
    case Qt::Key_I:
        if (pressed)
            ula->setKeyDown(5, 2);
        else
            ula->setKeyUp(5, 2); // I
        break;
    case Qt::Key_U:
        if (pressed)
            ula->setKeyDown(5, 3);
        else
            ula->setKeyUp(5, 3); // U
        break;
    case Qt::Key_Y:
        if (pressed)
            ula->setKeyDown(5, 4);
        else
            ula->setKeyUp(5, 4); // Y
        break;

    // Row 6: ENTER, L, K, J, H
    case Qt::Key_Enter:
        if (pressed)
            ula->setKeyDown(6, 0);
        else
            ula->setKeyUp(6, 0); // ENTER
        break;
    case Qt::Key_L:
        if (pressed)
            ula->setKeyDown(6, 1);
        else
            ula->setKeyUp(6, 1); // L
        break;
    case Qt::Key_K:
        if (pressed)
            ula->setKeyDown(6, 2);
        else
            ula->setKeyUp(6, 2); // K
        break;
    case Qt::Key_J:
        if (pressed)
            ula->setKeyDown(6, 3);
        else
            ula->setKeyUp(6, 3); // J
        break;
    case Qt::Key_H:
        if (pressed)
            ula->setKeyDown(6, 4);
        else
            ula->setKeyUp(6, 4); // H
        break;

    // Row 7: SPACE, SYM SHIFT, M, N, B
    case Qt::Key_Space:
        if (pressed)
            ula->setKeyDown(7, 0);
        else
            ula->setKeyUp(7, 0); // SPACE
        break;
    case Qt::Key_M:
        if (pressed)
            ula->setKeyDown(7, 2);
        else
            ula->setKeyUp(7, 2); // M
        break;
    case Qt::Key_N:
        if (pressed)
            ula->setKeyDown(7, 3);
        else
            ula->setKeyUp(7, 3); // N
        break;
    case Qt::Key_B:
        if (pressed)
            ula->setKeyDown(7, 4);
        else
            ula->setKeyUp(7, 4); // B
        break;
    default:
        handled = false;
    }
    return handled;
}
