#include <QKeyEvent>
#include <SDL3/SDL.h>
#include "emu.h"

Emu::Emu() {
    memory = std::make_unique<Memory>();
    ports = std::make_unique<Port>();
    cpu = std::make_unique<Z80>(memory.get(), ports.get());
    tape = std::make_unique<Tape>();
    ula = std::make_unique<ULA>(memory.get(), tape.get());
    kempston = std::make_unique<Kempston>();

    ports->RegisterReadHandler(0x1F, [this](uint16_t port) -> uint8_t
                               { return kempston->readPort(port); });


    if (!SDL_Init(SDL_INIT_AUDIO))
    {
        qDebug() << "SDL could not initialize! SDL_Error: " << SDL_GetError();
        qDebug() << "SDL Error code: " << SDL_GetError();
    }

    sound = std::make_unique<Sound>();
    if (!sound->initialize())
    {
        qDebug() << "Warning: Failed to initialize beeper system";
    }

    ay8912 = std::make_unique<AY8912>();
    if (!ay8912->initialize())
    {
        qDebug() << "Warning: Failed to initialize AY8912 sound chip";
    }

    // Connect AY8912 to port 0xFD (shared with memory)
    ports->RegisterWriteHandler(0xFD, [this](uint16_t port, uint8_t value)
                                { ay8912->writePort(port, value); });
    ports->RegisterReadHandler(0xFD, [this](uint16_t port) -> uint8_t
                               { return ay8912->readPort(port); });

    cpu->isNMOS = false;
    cpuSpeed = 3500000;
    CHECK_INTERVAL = cpuSpeed / 10000;
    busDelimeter = 1;
    memoryType = 0;
    ulaType = 0;
}

void Emu::Reset() {
    ports->Clear();
    cpu->Reset();
    memory->Clear();
    setMemoryType(memoryType);
    setULAType(ulaType);
    qDebug() << "Z80 speed:" << cpuSpeed << "bus delimeter:" << busDelimeter;
    qDebug() << "Memory type:" << memoryType << "ULA type:" << ulaType;

    // ULA always works
    ports->RegisterReadHandler(0xFE, [this](uint16_t port) -> uint8_t
                               { return ula->readPort(port); });
    ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value)
                                { ula->writePort(port, value); });
    // Beeper work always too
    ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value)
                                { sound->writePort(port, value); });


}

uint32_t *Emu::getScreenBuffer(void){
    return ula->getScreenBuffer();
}

void Emu::setMemoryType(uint type)
{
    memoryType = type;
    switch (type) {
    case ZX_SPECTRUM_48:
        memory->change48(true);
        break;
    case ZX_SPECTRUM_128:
        memory->change48(false);
        ports->RegisterWriteHandler(0xFD, [this](uint16_t port, uint8_t value)
                                    { return memory->writePort(port, value); });
        break;
    default:
        break;
    }
}

void Emu::setULAType(uint type)
{
    ulaType = type;
    switch (type) {
    case ZX_SPECTRUM_48:
        ula->change48(true);
        break;
    case ZX_SPECTRUM_128:
        ula->change48(false);
        break;
    default:
        break;
    }
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

void Emu::Run(void)
{
    // Signal that the emulation thread should be running
    threadRunning = true;

    // Create a new thread to run the CPU emulation
    // This allows the UI to remain responsive while the CPU emulation runs
    emulationThread = std::thread([this]()
                                  {
                                      // Timing variables for maintaining accurate CPU speed
                                      auto startTime = std::chrono::high_resolution_clock::now();
                                      long long totalTicks = 0; // Total CPU cycles executed
                                      long long checkTicks = 0; // Cycles since last timing check

                                      // Track previous tape state to detect when turbo mode turns off
                                      bool prevTapePlayed = false;
                                      bool prevTapeTurbo = false;

                                      // Main emulation loop - runs until threadRunning is set to false
                                      while (threadRunning.load())
                                      {
                                          // Execute one instruction and get the number of CPU cycles it took
                                          int ticks = cpu->ExecuteOneInstruction();
                                          // TR-DOS enable/disable block
                                          if(cpu->PC >= 0x3d00 && cpu->PC <= 0x3dff && memory->checkTrDos() == false) {
                                              memory->enableTrDos(true);
                                              //printf("TRDOS enable\n");
                                          }
                                          if(cpu->PC > 0x3fff && memory->checkTrDos() == true)
                                          {
                                              memory->enableTrDos(false);
                                              //printf("TRDOS disable\n");
                                          }

                                          // Update our cycle counters
                                          totalTicks += ticks;
                                          checkTicks += ticks;

                                          // Update sound system with current cycle count
                                          // This ensures audio stays synchronized with the CPU
                                          sound->ticks = totalTicks;

                                          // Update screen for each CPU tick
                                          // The ULA (graphics chip) needs to be updated for each cycle
                                          for (int i = 0; i < ticks; i++)
                                          {
                                              // oneTick() returns 0 when the screen is fully drawn
                                              int ref = ula->oneTick();
                                              if (ref == 0)
                                              {
                                                  // Screen has been updated - notify the main thread
                                                  {
                                                      // Rate limiting for screen updates during tape turbo mode
                                                      // This prevents the UI from being overwhelmed during fast tape loading
                                                      if (!tape->isTapePlayed || !tape->isTapeTurbo)
                                                      {
                                                          // Normal operation - update screen immediately
                                                          updateScreen();
                                                      }
                                                      else
                                                      {
                                                          // Turbo mode - limit screen updates to prevent UI lag
                                                          auto now = std::chrono::high_resolution_clock::now();
                                                          auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastScreenUpdate);
                                                          if (elapsed >= minScreenUpdateInterval)
                                                          {
                                                              updateScreen();
                                                              lastScreenUpdate = now;
                                                          }
                                                      }

                                                      // Signal that an interrupt should be triggered
                                                      // This is part of the ZX Spectrum's timing system
                                                      cpu->InterruptPending = true;
                                                  }
                                              }
                                          }

                                          // Detect transition from turbo mode to normal mode
                                          // When this happens, we need to reset our timing calculations
                                          if ((prevTapePlayed != tape->isTapePlayed || prevTapeTurbo != tape->isTapeTurbo) &&
                                              (!tape->isTapePlayed || !tape->isTapeTurbo))
                                          {
                                              // Reset timing when transitioning to normal mode
                                              startTime = std::chrono::high_resolution_clock::now();
                                              totalTicks = 0;
                                              checkTicks = 0;
                                              qDebug() << "Speed limiter re-enabled after tape play";
                                          }

                                          // Update previous states for next iteration
                                          prevTapePlayed = tape->isTapePlayed;
                                          prevTapeTurbo = tape->isTapeTurbo;

                                          // Determine if we should apply speed limiting
                                          // Speed limiting is disabled during tape turbo mode for faster loading
                                          bool shouldDisableLimiter = !tape->isTapePlayed || !tape->isTapeTurbo;

                                          // Apply speed limiting to maintain accurate CPU frequency
                                          // Without this, the emulator would run as fast as possible
                                          if (shouldDisableLimiter)
                                          {
                                              // Check timing periodically (every CHECK_INTERVAL cycles)
                                              if (checkTicks >= CHECK_INTERVAL)
                                              {
                                                  checkTicks = 0;

                                                  // Calculate how much time has actually passed
                                                  auto currentTime = std::chrono::high_resolution_clock::now();
                                                  auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - startTime).count();

                                                  // Calculate how much time should have passed for the number of cycles executed
                                                  long long expectedTime = (totalTicks * 1000000000LL) / cpuSpeed;

                                                  // If we're ahead of schedule, we need to wait
                                                  if (expectedTime > elapsedTime)
                                                  {
                                                      // Calculate how long to wait (in microseconds)
                                                      long long sleepTime = (expectedTime - elapsedTime) / 1000;

                                                      if (sleepTime > 1000)
                                                      {
                                                          // For longer delays, use sleep (less precise but CPU-friendly)
                                                          std::this_thread::sleep_for(std::chrono::microseconds(sleepTime - 500));
                                                          // Fine-tune with busy waiting for accuracy
                                                          while (std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                                     std::chrono::high_resolution_clock::now() - startTime)
                                                                     .count() < expectedTime)
                                                          {
                                                              std::this_thread::yield();
                                                          }
                                                      }
                                                      else if (sleepTime > 0)
                                                      {
                                                          // For short delays, use busy waiting for precision
                                                          while (std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                                     std::chrono::high_resolution_clock::now() - startTime)
                                                                     .count() < expectedTime)
                                                          {
                                                              std::this_thread::yield();
                                                          }
                                                      }
                                                  }
                                              }
                                          }
                                      } // End of main emulation loop
                                  }); // End of thread creation
}
