#include <SDL3/SDL.h>
#include <iostream>
#include <memory>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include "ula.hpp"
#include "memory.hpp"
#include "port.hpp"
#include "z80.hpp"
#include "kempston.hpp"
#include "sound.hpp"
#include "tape.hpp"
#include "ay8912.hpp"

// ImGui includes
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "ImGuiFileDialog.h"

class Emulator
{
private:
    // SDL graphics components
    SDL_Window *window;     // Main window for the emulator
    SDL_Renderer *renderer; // Renderer for drawing graphics
    SDL_Texture *texture;   // Texture for the screen buffer
    bool quit;              // Flag to signal when to exit the emulator

    // Thread management for running the emulation in the background
    std::thread emulationThread;     // Thread that runs the Z80 CPU emulation
    std::atomic<bool> threadRunning; // Atomic flag to safely control thread execution

    // Emulator hardware components (each represents a real ZX Spectrum chip or subsystem)
    std::unique_ptr<Memory> memory;     // RAM and ROM memory management
    std::unique_ptr<Port> ports;        // Input/output port handling
    std::unique_ptr<Z80> cpu;           // Zilog Z80 CPU emulation
    std::unique_ptr<ULA> ula;           // Sinclair ULA (graphics and keyboard controller)
    std::unique_ptr<Kempston> kempston; // Kempston joystick interface
    std::unique_ptr<Sound> sound;       // Beeper sound system
    std::unique_ptr<Tape> tape;         // Tape loading system
    std::unique_ptr<AY8912> ay8912;     // AY-3-8912 sound chip (for better sound)

    // Thread synchronization for safely sharing data between threads
    std::mutex screenMutex; // Mutex to protect screen data when updating from different threads
    bool screenUpdated;     // Flag to indicate when the screen has been updated

    // Screen update rate limiting to prevent overwhelming the display during fast operations
    std::chrono::high_resolution_clock::time_point lastScreenUpdate; // Last time screen was updated
    const std::chrono::milliseconds minScreenUpdateInterval{100};    // Minimum time between screen updates (10 FPS)

    // Keyboard handling functions
    void handleKeyDown(SDL_Keycode key); // Process key press events
    void handleKeyUp(SDL_Keycode key);   // Process key release events

    // Change machine type: true for 48k, false for 128k
    void change48(bool is48);

    // Start tape playback
    void StartTape();

    // CPU timing parameters
    int TARGET_FREQUENCY; // Target CPU frequency in Hz
    int CHECK_INTERVAL;   // How often to check timing (in CPU cycles)

public:
    // Constructor - initializes all pointers to null/false
    // This is called when an Emulator object is created
    Emulator()
    {
        // Initialize all SDL components to null pointers
        // These will be properly allocated in the initialize() method
        window = nullptr;
        renderer = nullptr;
        texture = nullptr;

        // Initialize flags to false
        // These control the state of our emulator
        quit = false;          // Not ready to quit yet
        threadRunning = false; // Emulation thread not running yet
        screenUpdated = false; // Screen hasn't been updated yet
    }

    // Run emulation in a separate thread
    void runZX();

    // Public methods for command line tape loading
    bool loadTapeFile(const std::string &filePath);
    void startTapePlayback();

    // Destructor - automatically cleans up when Emulator object is destroyed
    ~Emulator()
    {
        cleanup();
    }

    bool initialize()
    {
        // Step 1: Initialize all core emulator components
        // These represent the actual hardware chips in a real ZX Spectrum

        // Initialize memory system (RAM and ROM)
        memory = std::make_unique<Memory>();

        // Initialize port system (input/output connections)
        ports = std::make_unique<Port>();

        // Initialize tape loading system
        tape = std::make_unique<Tape>();

        // Initialize main processor (Z80 CPU)
        // Pass references to memory and ports so CPU can interact with them
        cpu = std::make_unique<Z80>(memory.get(), ports.get());

        // Initialize graphics and keyboard controller (ULA chip)
        // Pass references to memory and tape systems
        ula = std::make_unique<ULA>(memory.get(), tape.get());

        // Initialize joystick interface (Kempston)
        kempston = std::make_unique<Kempston>();

        // Step 2: Connect hardware components through port handlers
        // The ZX Spectrum uses specific port addresses to communicate with hardware

        // Connect ULA (graphics/keyboard controller) to port 0xFE
        ports->RegisterReadHandler(0xFE, [this](uint16_t port) -> uint8_t
                                   { return ula->readPort(port); });
        ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value)
                                    { ula->writePort(port, value); });

        // Connect Kempston joystick to port 0x1F
        ports->RegisterReadHandler(0x1F, [this](uint16_t port) -> uint8_t
                                   { return kempston->readPort(port); });

        // Connect Memory interface to port 0xFD
        ports->RegisterWriteHandler(0xFD, [this](uint16_t port, uint8_t value)
                                    { return memory->writePort(port, value); });

        // Step 3: Initialize SDL for graphics and sound
        // SDL is a cross-platform library for multimedia applications
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            std::cerr << "SDL Error code: " << SDL_GetError() << std::endl;
            return false;
        }

        // Step 4: Initialize sound systems
        // Set up both the basic beeper and advanced AY-3-8912 sound chip

        // Initialize basic beeper sound system
        sound = std::make_unique<Sound>();
        if (!sound->initialize())
        {
            std::cerr << "Warning: Failed to initialize sound system" << std::endl;
            // Continue without sound if initialization fails
        }

        // Connect beeper to port 0xFE (shared with ULA)
        ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value)
                                    { sound->writePort(port, value); });

        // Initialize AY8912 sound chip (provides better sound quality)
        ay8912 = std::make_unique<AY8912>();
        if (!ay8912->initialize())
        {
            std::cerr << "Warning: Failed to initialize AY8912 sound chip" << std::endl;
            // Continue without AY8912 sound if initialization fails
        }

        // Connect AY8912 to port 0xFD (shared with memory)
        ports->RegisterWriteHandler(0xFD, [this](uint16_t port, uint8_t value)
                                    { ay8912->writePort(port, value); });
        ports->RegisterReadHandler(0xFD, [this](uint16_t port) -> uint8_t
                                   { return ay8912->readPort(port); });

        // Step 5: Create graphics window and rendering components
        window = SDL_CreateWindow("ZX Spectrum Emulator", 704, 576, SDL_WINDOW_RESIZABLE);
        if (window == nullptr)
        {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Show the window on screen
        SDL_ShowWindow(window);

        // Create renderer for drawing graphics
        renderer = SDL_CreateRenderer(window, nullptr);
        if (renderer == nullptr)
        {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create texture for the screen (352x288 pixels matches the ULA output buffer)
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STATIC, 352, 288);
        if (texture == nullptr)
        {
            std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

        // Step 6: Initialize ImGui for the user interface
        IMGUI_CHECKVERSION();   // Verify ImGui version compatibility
        ImGui::CreateContext(); // Create ImGui context
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation

        // Setup ImGui visual style
        ImGui::StyleColorsDark();

        // Setup platform/renderer backends for ImGui
        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        // Default to 48k Spectrum mode
        change48(true);
        return true;
    }

    void run()
    {
        // Event structure for handling SDL events (keyboard, mouse, window events, etc.)
        SDL_Event e;

        // Start the CPU emulation in a separate thread
        // This allows the UI to remain responsive while the CPU emulation runs
        runZX();

        // Main application loop - continues until quit flag is set
        while (!quit)
        {
            // Handle all pending events in the queue
            // Events include keyboard presses, mouse movements, window resizing, etc.
            while (SDL_PollEvent(&e) != 0)
            {
                // Pass SDL events to ImGui for processing (menu interactions, etc.)
                ImGui_ImplSDL3_ProcessEvent(&e);

                // Handle different types of events
                if (e.type == SDL_EVENT_QUIT)
                {
                    // User closed the window - clean up and exit
                    ay8912->cleanup();
                    sound->cleanup();
                    std::cout << "Quit event received" << std::endl;
                    quit = true;
                }
                else if (e.type == SDL_EVENT_KEY_DOWN)
                {
                    // Key pressed down - handle keyboard input
                    handleKeyDown(e.key.key);
                }
                else if (e.type == SDL_EVENT_KEY_UP)
                {
                    // Key released - handle keyboard input
                    handleKeyUp(e.key.key);
                }
            }

            // Start a new ImGui frame for UI rendering
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            // Create the main menu bar at the top of the window
            if (ImGui::BeginMainMenuBar())
            {
                // File menu - for loading files and exiting
                if (ImGui::BeginMenu("File"))
                {
                    // Option to open a ROM/BIN/Z80 file
                    if (ImGui::MenuItem("Open File", "Ctrl+O"))
                    {
                        // Configure and open file dialog for ROM files
                        IGFD::FileDialogConfig config;
                        config.path = "."; // Start in current directory
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".rom,.bin,.z80", config);
                    }

                    // Exit option
                    if (ImGui::MenuItem("Exit", "Alt+F4"))
                    {
                        quit = true;
                    }
                    ImGui::EndMenu();
                }

                // Window menu - for changing display scale
                if (ImGui::BeginMenu("Window"))
                {
                    // Scale options to change window size
                    if (ImGui::MenuItem("Scale 1x"))
                    {
                        // Set window size to original resolution (352x288)
                        SDL_SetWindowMinimumSize(window, 352, 288);
                        SDL_SetWindowSize(window, 352, 288);
                    }
                    if (ImGui::MenuItem("Scale 2x"))
                    {
                        // Double the window size (704x576)
                        SDL_SetWindowMinimumSize(window, 704, 576);
                        SDL_SetWindowSize(window, 704, 576);
                    }
                    if (ImGui::MenuItem("Scale 3x"))
                    {
                        // Triple the window size (1056x864)
                        SDL_SetWindowMinimumSize(window, 1056, 864);
                        SDL_SetWindowSize(window, 1056, 864);
                    }
                    ImGui::EndMenu();
                }

                // Tape menu - for loading and controlling tape files
                if (ImGui::BeginMenu("Tape"))
                {
                    // Load a tape file (.TAP or .TZX format)
                    if (ImGui::MenuItem("Load"))
                    {
                        // Configure and open file dialog for tape files
                        IGFD::FileDialogConfig config;
                        config.path = "."; // Start in current directory
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseTapeDlgKey", "Choose Tape File", ".TAP,.TZX,.tap,.tzx", config);
                    }

                    // Start playing the currently loaded tape
                    if (ImGui::MenuItem("Play"))
                    {
                        StartTape();
                    }

                    // Toggle turbo loading mode (faster tape loading)
                    if (ImGui::MenuItem("Turboload", nullptr, tape ? tape->isTapeTurbo : false))
                    {
                        if (tape)
                        {
                            tape->isTapeTurbo = !tape->isTapeTurbo;
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // Display file dialog for ROM loading if it's open
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(400, 300)))
            {
                // If user clicked OK in the dialog
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // Get the selected file path
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                    // TODO: Actually load the selected ROM file
                    std::cout << "Selected file: " << filePathName << std::endl;
                }

                // Close the file dialog
                ImGuiFileDialog::Instance()->Close();
            }

            // Display file dialog for tape loading if it's open
            if (ImGuiFileDialog::Instance()->Display("ChooseTapeDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(400, 300)))
            {
                // If user clicked OK in the dialog
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // Get the selected tape file path
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                    // Load the selected tape file
                    std::cout << "Selected tape file: " << filePathName << std::endl;
                    if (tape)
                    {
                        tape->loadFile(filePathName);
                        tape->prepareBitStream();
                    }
                }

                // Close the file dialog
                ImGuiFileDialog::Instance()->Close();
            }

            // Check if the emulation thread has updated the screen
            // We need to synchronize access to shared data using a mutex
            bool updateScreen = false;
            {
                // Lock the mutex to safely check screen update flag
                std::lock_guard<std::mutex> lock(screenMutex);
                if (screenUpdated)
                {
                    updateScreen = true;
                    screenUpdated = false; // Reset the flag
                }
            }

            // Update the screen texture if the emulation thread has rendered a new frame
            if (updateScreen)
            {
                // Get the screen buffer from the ULA (graphics chip)
                // This must be done on the main thread because OpenGL/DirectX isn't thread-safe
                uint32_t *src = ula->getScreenBuffer();

                // Update the SDL texture with the new screen data
                SDL_UpdateTexture(texture, nullptr, src, 352 * sizeof(uint32_t));
            }

            // Get current window dimensions for scaling calculations
            int windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            // Update ImGui with current display size
            ImGuiIO &io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)windowWidth, (float)windowHeight);

            // Clear the screen with black to prevent flickering between frames
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Calculate scaling to maintain correct aspect ratio
            float scaleX = (float)windowWidth / 352.0f;  // Original width
            float scaleY = (float)windowHeight / 288.0f; // Original height
            float scale = std::min(scaleX, scaleY);      // Use the smaller scale to fit entirely

            // Calculate destination rectangle for scaled rendering
            int destWidth = (int)(352.0f * scale);
            int destHeight = (int)(288.0f * scale);
            int destX = (windowWidth - destWidth) / 2;   // Center horizontally
            int destY = (windowHeight - destHeight) / 2; // Center vertically

            // Create rectangle structure for rendering
            SDL_FRect destRect = {(float)destX, (float)destY, (float)destWidth, (float)destHeight};

            // Render the scaled texture to the screen
            SDL_RenderTexture(renderer, texture, nullptr, &destRect);

            // Render ImGui elements (menus, dialogs, etc.)
            ImGui::Render();
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

            // Present the final rendered frame to the screen
            SDL_RenderPresent(renderer);
        }
    }

    void cleanup()
    {
        sound->cleanup();
        // Stop the thread if it's running
        if (threadRunning.load())
        {
            threadRunning = false;
            if (emulationThread.joinable())
            {
                emulationThread.join();
            }
        }

        // Cleanup ImGui
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        // Cleanup SDL resources
        if (texture)
        {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        if (renderer)
        {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }

        if (window)
        {
            SDL_DestroyWindow(window);
            window = nullptr;
        }

        SDL_Quit();
    }
};

void Emulator::change48(bool is48)
{
    if (is48)
    {
        TARGET_FREQUENCY = 3500000;                // 3.5 MHz
        CHECK_INTERVAL = TARGET_FREQUENCY / 10000; // Check timing every 0.1ms (more precise)
        ula->change48(true);
    }
    else
    {
        TARGET_FREQUENCY = 3546900;                // 3.54690 Mhz
        CHECK_INTERVAL = TARGET_FREQUENCY / 10000; // Check timing every 0.1ms (more precise)
        ula->change48(false);
    }
}

void Emulator::StartTape()
{
    // Empty function as requested
    std::cout << "StartTape() called" << std::endl;

    if (tape)
    {
        // Set the tape to played state
        tape->isTapePlayed = true;
        std::cout << "Tape playback started" << std::endl;
    }
}

bool Emulator::loadTapeFile(const std::string &filePath)
{
    if (!tape)
    {
        std::cerr << "Tape system not initialized" << std::endl;
        return false;
    }

    std::cout << "Loading tape file: " << filePath << std::endl;
    tape->loadFile(filePath);
    tape->prepareBitStream();
    std::cout << "Tape file loaded successfully" << std::endl;
    return true;
}

void Emulator::startTapePlayback()
{
    if (tape)
    {
        tape->isTapePlayed = true;
        std::cout << "Tape playback started automatically" << std::endl;
    }
}

void Emulator::runZX()
{
    // Signal that the emulation thread should be running
    threadRunning = true;

    // Initialize memory with 128K ROM (default mode)
    // The ZX Spectrum 128K had more memory and additional features compared to the 48K model
    memory->Read128();       // Load 128K ROM
    //memory->Read48();
    memory->change48(false); // Set memory mode to 128K

    // Set CPU to CMOS mode (more accurate for later Spectrums)
    // The Z80 CPU in later Spectrum models was a CMOS variant
    cpu->isNMOS = false;

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
                                            printf("TRDOS enable\n");
                                          }
                                          if(cpu->PC > 0x3fff && memory->checkTrDos() == true)
                                          {
                                            memory->enableTrDos(false);
                                            printf("TRDOS disable\n");
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
                                                      // Lock the mutex to safely update shared data
                                                      std::lock_guard<std::mutex> lock(screenMutex);

                                                      // Rate limiting for screen updates during tape turbo mode
                                                      // This prevents the UI from being overwhelmed during fast tape loading
                                                      if (!tape->isTapePlayed || !tape->isTapeTurbo)
                                                      {
                                                          // Normal operation - update screen immediately
                                                          screenUpdated = true;
                                                      }
                                                      else
                                                      {
                                                          // Turbo mode - limit screen updates to prevent UI lag
                                                          auto now = std::chrono::high_resolution_clock::now();
                                                          auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastScreenUpdate);
                                                          if (elapsed >= minScreenUpdateInterval)
                                                          {
                                                              screenUpdated = true;
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
                                              std::cout << "Speed limiter re-enabled after tape play" << std::endl;
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
                                                  long long expectedTime = (totalTicks * 1000000000LL) / TARGET_FREQUENCY;

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

// Helper function to handle Kempston joystick events
void handleKempstonJoystick(SDL_Keycode key, bool pressed, std::unique_ptr<Kempston> &kempston)
{
    // Only process if Kempston joystick is available
    if (!kempston)
        return;

    // Map arrow keys and Alt to joystick directions and fire button
    switch (key)
    {
    case SDLK_UP:
        kempston->setUp(pressed);
        break;
    case SDLK_DOWN:
        kempston->setDown(pressed);
        break;
    case SDLK_LEFT:
        kempston->setLeft(pressed);
        break;
    case SDLK_RIGHT:
        kempston->setRight(pressed);
        break;
    case SDLK_LALT:
        kempston->setFire(pressed);
        break;
    }
}

// Helper function to map SDL keys to ZX Spectrum keyboard matrix rows and columns
void mapKeyToSpectrum(SDL_Keycode key, bool pressed, ULA *ula)
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

    switch (key)
    {
    // Row 0: CAPS SHIFT, Z, X, C, V
    case SDLK_LSHIFT:
        if (pressed)
            ula->setKeyDown(0, 0);
        else
            ula->setKeyUp(0, 0); // CAPS SHIFT
        break;
    case SDLK_Z:
        if (pressed)
            ula->setKeyDown(0, 1);
        else
            ula->setKeyUp(0, 1); // Z
        break;
    case SDLK_X:
        if (pressed)
            ula->setKeyDown(0, 2);
        else
            ula->setKeyUp(0, 2); // X
        break;
    case SDLK_C:
        if (pressed)
            ula->setKeyDown(0, 3);
        else
            ula->setKeyUp(0, 3); // C
        break;
    case SDLK_V:
        if (pressed)
            ula->setKeyDown(0, 4);
        else
            ula->setKeyUp(0, 4); // V
        break;

    // Row 1: A, S, D, F, G
    case SDLK_A:
        if (pressed)
            ula->setKeyDown(1, 0);
        else
            ula->setKeyUp(1, 0); // A
        break;
    case SDLK_S:
        if (pressed)
            ula->setKeyDown(1, 1);
        else
            ula->setKeyUp(1, 1); // S
        break;
    case SDLK_D:
        if (pressed)
            ula->setKeyDown(1, 2);
        else
            ula->setKeyUp(1, 2); // D
        break;
    case SDLK_F:
        if (pressed)
            ula->setKeyDown(1, 3);
        else
            ula->setKeyUp(1, 3); // F
        break;
    case SDLK_G:
        if (pressed)
            ula->setKeyDown(1, 4);
        else
            ula->setKeyUp(1, 4); // G
        break;

    // Row 2: Q, W, E, R, T
    case SDLK_Q:
        if (pressed)
            ula->setKeyDown(2, 0);
        else
            ula->setKeyUp(2, 0); // Q
        break;
    case SDLK_W:
        if (pressed)
            ula->setKeyDown(2, 1);
        else
            ula->setKeyUp(2, 1); // W
        break;
    case SDLK_E:
        if (pressed)
            ula->setKeyDown(2, 2);
        else
            ula->setKeyUp(2, 2); // E
        break;
    case SDLK_R:
        if (pressed)
            ula->setKeyDown(2, 3);
        else
            ula->setKeyUp(2, 3); // R
        break;
    case SDLK_T:
        if (pressed)
            ula->setKeyDown(2, 4);
        else
            ula->setKeyUp(2, 4); // T
        break;

    // Row 3: 1, 2, 3, 4, 5
    case SDLK_1:
        if (pressed)
            ula->setKeyDown(3, 0);
        else
            ula->setKeyUp(3, 0); // 1
        break;
    case SDLK_2:
        if (pressed)
            ula->setKeyDown(3, 1);
        else
            ula->setKeyUp(3, 1); // 2
        break;
    case SDLK_3:
        if (pressed)
            ula->setKeyDown(3, 2);
        else
            ula->setKeyUp(3, 2); // 3
        break;
    case SDLK_4:
        if (pressed)
            ula->setKeyDown(3, 3);
        else
            ula->setKeyUp(3, 3); // 4
        break;
    case SDLK_5:
        if (pressed)
            ula->setKeyDown(3, 4);
        else
            ula->setKeyUp(3, 4); // 5
        break;

    // Row 4: 0, 9, 8, 7, 6
    case SDLK_0:
        if (pressed)
            ula->setKeyDown(4, 0);
        else
            ula->setKeyUp(4, 0); // 0
        break;
    case SDLK_9:
        if (pressed)
            ula->setKeyDown(4, 1);
        else
            ula->setKeyUp(4, 1); // 9
        break;
    case SDLK_8:
        if (pressed)
            ula->setKeyDown(4, 2);
        else
            ula->setKeyUp(4, 2); // 8
        break;
    case SDLK_7:
        if (pressed)
            ula->setKeyDown(4, 3);
        else
            ula->setKeyUp(4, 3); // 7
        break;
    case SDLK_6:
        if (pressed)
            ula->setKeyDown(4, 4);
        else
            ula->setKeyUp(4, 4); // 6
        break;

    // Row 5: P, O, I, U, Y
    case SDLK_P:
        if (pressed)
            ula->setKeyDown(5, 0);
        else
            ula->setKeyUp(5, 0); // P
        break;
    case SDLK_O:
        if (pressed)
            ula->setKeyDown(5, 1);
        else
            ula->setKeyUp(5, 1); // O
        break;
    case SDLK_I:
        if (pressed)
            ula->setKeyDown(5, 2);
        else
            ula->setKeyUp(5, 2); // I
        break;
    case SDLK_U:
        if (pressed)
            ula->setKeyDown(5, 3);
        else
            ula->setKeyUp(5, 3); // U
        break;
    case SDLK_Y:
        if (pressed)
            ula->setKeyDown(5, 4);
        else
            ula->setKeyUp(5, 4); // Y
        break;

    // Row 6: ENTER, L, K, J, H
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        if (pressed)
            ula->setKeyDown(6, 0);
        else
            ula->setKeyUp(6, 0); // ENTER
        break;
    case SDLK_L:
        if (pressed)
            ula->setKeyDown(6, 1);
        else
            ula->setKeyUp(6, 1); // L
        break;
    case SDLK_K:
        if (pressed)
            ula->setKeyDown(6, 2);
        else
            ula->setKeyUp(6, 2); // K
        break;
    case SDLK_J:
        if (pressed)
            ula->setKeyDown(6, 3);
        else
            ula->setKeyUp(6, 3); // J
        break;
    case SDLK_H:
        if (pressed)
            ula->setKeyDown(6, 4);
        else
            ula->setKeyUp(6, 4); // H
        break;

    // Row 7: SPACE, SYM SHIFT, M, N, B
    case SDLK_SPACE:
        if (pressed)
            ula->setKeyDown(7, 0);
        else
            ula->setKeyUp(7, 0); // SPACE
        break;
    case SDLK_RSHIFT:
        if (pressed)
            ula->setKeyDown(7, 1);
        else
            ula->setKeyUp(7, 1); // SYM SHIFT
        break;
    case SDLK_M:
        if (pressed)
            ula->setKeyDown(7, 2);
        else
            ula->setKeyUp(7, 2); // M
        break;
    case SDLK_N:
        if (pressed)
            ula->setKeyDown(7, 3);
        else
            ula->setKeyUp(7, 3); // N
        break;
    case SDLK_B:
        if (pressed)
            ula->setKeyDown(7, 4);
        else
            ula->setKeyUp(7, 4); // B
        break;
    }
}

// Handle key down events
void Emulator::handleKeyDown(SDL_Keycode key)
{
    // Handle Kempston joystick with arrow keys and Alt
    handleKempstonJoystick(key, true, kempston);

    // Map SDL keys to ZX Spectrum keyboard matrix
    mapKeyToSpectrum(key, true, ula.get());
}

// Handle key up events
void Emulator::handleKeyUp(SDL_Keycode key)
{
    // Handle Kempston joystick with arrow keys and Alt
    handleKempstonJoystick(key, false, kempston);

    // Map SDL keys to ZX Spectrum keyboard matrix
    mapKeyToSpectrum(key, false, ula.get());
}

// Main entry point of the program
// This is where execution begins when you start the emulator
int main(int argc, char *argv[])
{
    // Welcome message for users
    std::cout << "ZX Spectrum Emulator starting..." << std::endl;

    // Create an instance of our Emulator class
    // This will initialize all the components we need
    // The constructor sets up default values, but doesn't allocate resources yet
    Emulator emulator;

    // Try to initialize the emulator
    // This sets up SDL, creates the window, loads ROMs, etc.
    // If this fails, we can't continue with the program
    std::cout << "Initializing emulator components..." << std::endl;
    if (!emulator.initialize())
    {
        std::cerr << "Failed to initialize emulator!" << std::endl;
        std::cerr << "The emulator could not start because of initialization errors." << std::endl;
        return -1; // Return error code if initialization fails
    }
    std::cout << "Emulator initialized successfully!" << std::endl;

    // Check if a file path was provided as a command line argument
    // This allows users to load a tape file directly when starting the emulator
    // Usage: ./emulator myfile.tap
    if (argc > 1)
    {
        // Get the file path from command line arguments
        // argv[0] is the program name, argv[1] is the first argument
        std::string filePath = argv[1];
        std::cout << "Loading tape file from command line: " << filePath << std::endl;

        // Load the tape file and prepare it for playback
        // This reads the file into memory and prepares it for the emulator to use
        if (emulator.loadTapeFile(filePath))
        {
            std::cout << "Tape file loaded successfully. Use Tape->Play menu to start playback." << std::endl;
        }
        else
        {
            std::cerr << "Failed to load tape file: " << filePath << std::endl;
            std::cerr << "The emulator will continue without the tape file." << std::endl;
        }
    }
    else
    {
        std::cout << "No tape file specified. You can load one using the Tape->Load menu." << std::endl;
    }

    // Start the main emulator loop
    // This will run until the user closes the window or exits the program
    // The run() method contains the main application loop that handles:
    // - Processing user input (keyboard, mouse, menus)
    // - Updating the display with the emulated screen
    // - Managing the emulation thread that runs the Z80 CPU
    std::cout << "Starting emulator loop..." << std::endl;
    emulator.run();

    // When run() finishes (user closed the window), the program ends
    std::cout << "Emulator shutting down..." << std::endl;

    // Program ends successfully
    std::cout << "Emulator exited normally." << std::endl;
    return 0;
}
