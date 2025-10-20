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

// ImGui includes
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

class Emulator
{
private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    bool quit;

    // Thread management
    std::thread emulationThread;
    std::atomic<bool> threadRunning;

    // Emulator components
    std::unique_ptr<Memory> memory;
    std::unique_ptr<Port> ports;
    std::unique_ptr<Z80> cpu;
    std::unique_ptr<ULA> ula;

    // Thread synchronization
    std::mutex screenMutex;
    bool screenUpdated;

    // Keyboard handling functions
    void handleKeyDown(SDL_Keycode key);
    void handleKeyUp(SDL_Keycode key);

public:
    Emulator() : window(nullptr), renderer(nullptr), texture(nullptr), quit(false), threadRunning(false), screenUpdated(false) {}

    // Run emulation in a separate thread
    void runZX();

    ~Emulator()
    {
        cleanup();
    }

    bool initialize()
    {
        // Initialize emulator components
        memory = std::make_unique<Memory>();
        ports = std::make_unique<Port>();
        cpu = std::make_unique<Z80>(memory.get(), ports.get());
        ula = std::make_unique<ULA>(memory.get());

        // Register ULA with port system
        ports->RegisterReadHandler(0xFE, [this](uint16_t port) -> uint8_t
                                   { return ula->readPort(port); });

        ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value)
                                    { ula->writePort(port, value); });

        // Initialize SDL
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            std::cerr << "SDL Error code: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create window
        window = SDL_CreateWindow("ZX Spectrum Emulator", 704, 576, SDL_WINDOW_RESIZABLE);
        if (window == nullptr)
        {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Show window
        SDL_ShowWindow(window);

        // Create renderer
        renderer = SDL_CreateRenderer(window, nullptr);
        if (renderer == nullptr)
        {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create texture for the screen (352x288 to match ULA buffer)
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STATIC, 352, 288);
        if (texture == nullptr)
        {
            std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        // Setup ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        return true;
    }

    void run()
    {
        SDL_Event e;
        std::cout << "Entering emulation loop" << std::endl;

        runZX();

        while (!quit)
        {
            // Handle events
            while (SDL_PollEvent(&e) != 0)
            {
                // Pass SDL events to ImGui
                ImGui_ImplSDL3_ProcessEvent(&e);
                
                if (e.type == SDL_EVENT_QUIT)
                {
                    std::cout << "Quit event received" << std::endl;
                    quit = true;
                }
                else if (e.type == SDL_EVENT_KEY_DOWN)
                {
                    handleKeyDown(e.key.key);
                }
                else if (e.type == SDL_EVENT_KEY_UP)
                {
                    handleKeyUp(e.key.key);
                }
            }

            // Start the ImGui frame
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            // Create a simple menu bar
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit", "Alt+F4"))
                    {
                        quit = true;
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("Window"))
                {
                    if (ImGui::MenuItem("Scale 1x")) {
                        // Set window size to 352x288
                        SDL_SetWindowMinimumSize(window, 352, 288);
                        SDL_SetWindowSize(window, 352, 288);
                    }
                    if (ImGui::MenuItem("Scale 2x")) {
                        // Set window size to 704x576
                        SDL_SetWindowMinimumSize(window, 704, 576);
                        SDL_SetWindowSize(window, 704, 576);
                    }
                    if (ImGui::MenuItem("Scale 3x")) {
                        // Set window size to 1056x864
                        SDL_SetWindowMinimumSize(window, 1056, 864);
                        SDL_SetWindowSize(window, 1056, 864);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // Check if screen was updated by emulation thread
            bool updateScreen = false;
            {
                std::lock_guard<std::mutex> lock(screenMutex);
                if (screenUpdated)
                {
                    updateScreen = true;
                    screenUpdated = false;
                }
            }

            if (updateScreen)
            {
                // Update screen with ULA output - this must be done on the main thread
                uint32_t *src = ula->getScreenBuffer();
                SDL_UpdateTexture(texture, nullptr, src, 352 * sizeof(uint32_t));
            }

            // Get window size
            int windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            // Update ImGui display size
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)windowWidth, (float)windowHeight);

            // Clear the screen with black color to prevent flickering
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Calculate destination rectangle to maintain aspect ratio
            float scaleX = (float)windowWidth / 352.0f;
            float scaleY = (float)windowHeight / 288.0f;
            float scale = std::min(scaleX, scaleY);

            int destWidth = (int)(352.0f * scale);
            int destHeight = (int)(288.0f * scale);
            int destX = (windowWidth - destWidth) / 2;
            int destY = (windowHeight - destHeight) / 2;

            SDL_FRect destRect = {(float)destX, (float)destY, (float)destWidth, (float)destHeight};

            // Render texture
            SDL_RenderTexture(renderer, texture, nullptr, &destRect);

            // Rendering ImGui
            ImGui::Render();
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

            // Update screen
            SDL_RenderPresent(renderer);
        }
    }

    void cleanup()
    {
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

void Emulator::runZX()
{
    threadRunning = true;
    memory->Read48();
    emulationThread = std::thread([this]()
                                  {
        const int TARGET_FREQUENCY = 3500000; // 3.5 MHz
        const int CHECK_INTERVAL = TARGET_FREQUENCY / 1000; // Check timing every 1ms
        auto startTime = std::chrono::high_resolution_clock::now();
        long long totalTicks = 0;
        long long checkTicks = 0;
        
        while (threadRunning.load()) {
            int ticks = cpu->ExecuteOneInstruction();
            totalTicks += ticks;
            checkTicks += ticks;
            
            for (int i = 0; i < ticks; i++) {
                int ref = ula->oneTick();
                if (ref == 0) {
                    // Signal that screen has been updated
                    {
                        std::lock_guard<std::mutex> lock(screenMutex);
                        screenUpdated = true;
                        cpu->InterruptPending = true;
                    }
                }
            }
            
            // Speed limiter - check timing more frequently for smoother execution
            if (checkTicks >= CHECK_INTERVAL) {
                checkTicks = 0;
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count();
                long long expectedTime = (totalTicks * 1000000LL) / TARGET_FREQUENCY;
                
                if (expectedTime > elapsedTime) {
                    long long sleepTime = expectedTime - elapsedTime;
                    // Limit sleep time to prevent large sleeps that could cause stuttering
                    if (sleepTime > 1000) { // Cap at 1ms
                        sleepTime = 1000;
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
                }
            }
        } });
}

// Handle key down events
void Emulator::handleKeyDown(SDL_Keycode key) {
    // Map SDL keys to ZX Spectrum keyboard matrix
    // ZX Spectrum keyboard matrix:
    // Half-row 0 (port 0xFEFE): CAPS SHIFT, Z, X, C, V
    // Half-row 1 (port 0xFDFE): A, S, D, F, G
    // Half-row 2 (port 0xFBFE): Q, W, E, R, T
    // Half-row 3 (port 0xF7FE): 1, 2, 3, 4, 5
    // Half-row 4 (port 0xEFFE): 0, 9, 8, 7, 6
    // Half-row 5 (port 0xDFFE): P, O, I, U, Y
    // Half-row 6 (port 0xBFFE): ENTER, L, K, J, H
    // Half-row 7 (port 0x7FFE): SPACE, SYM SHIFT, M, N, B
    
    switch (key) {
        // Row 0: CAPS SHIFT, Z, X, C, V
        case SDLK_LSHIFT:
            ula->setKeyDown(0, 0); // CAPS SHIFT
            break;
        case SDLK_Z:
            ula->setKeyDown(0, 1); // Z
            break;
        case SDLK_X:
            ula->setKeyDown(0, 2); // X
            break;
        case SDLK_C:
            ula->setKeyDown(0, 3); // C
            break;
        case SDLK_V:
            ula->setKeyDown(0, 4); // V
            break;
            
        // Row 1: A, S, D, F, G
        case SDLK_A:
            ula->setKeyDown(1, 0); // A
            break;
        case SDLK_S:
            ula->setKeyDown(1, 1); // S
            break;
        case SDLK_D:
            ula->setKeyDown(1, 2); // D
            break;
        case SDLK_F:
            ula->setKeyDown(1, 3); // F
            break;
        case SDLK_G:
            ula->setKeyDown(1, 4); // G
            break;
            
        // Row 2: Q, W, E, R, T
        case SDLK_Q:
            ula->setKeyDown(2, 0); // Q
            break;
        case SDLK_W:
            ula->setKeyDown(2, 1); // W
            break;
        case SDLK_E:
            ula->setKeyDown(2, 2); // E
            break;
        case SDLK_R:
            ula->setKeyDown(2, 3); // R
            break;
        case SDLK_T:
            ula->setKeyDown(2, 4); // T
            break;
            
        // Row 3: 1, 2, 3, 4, 5
        case SDLK_1:
            ula->setKeyDown(3, 0); // 1
            break;
        case SDLK_2:
            ula->setKeyDown(3, 1); // 2
            break;
        case SDLK_3:
            ula->setKeyDown(3, 2); // 3
            break;
        case SDLK_4:
            ula->setKeyDown(3, 3); // 4
            break;
        case SDLK_5:
            ula->setKeyDown(3, 4); // 5
            break;
            
        // Row 4: 0, 9, 8, 7, 6
        case SDLK_0:
            ula->setKeyDown(4, 0); // 0
            break;
        case SDLK_9:
            ula->setKeyDown(4, 1); // 9
            break;
        case SDLK_8:
            ula->setKeyDown(4, 2); // 8
            break;
        case SDLK_7:
            ula->setKeyDown(4, 3); // 7
            break;
        case SDLK_6:
            ula->setKeyDown(4, 4); // 6
            break;
            
        // Row 5: P, O, I, U, Y
        case SDLK_P:
            ula->setKeyDown(5, 0); // P
            break;
        case SDLK_O:
            ula->setKeyDown(5, 1); // O
            break;
        case SDLK_I:
            ula->setKeyDown(5, 2); // I
            break;
        case SDLK_U:
            ula->setKeyDown(5, 3); // U
            break;
        case SDLK_Y:
            ula->setKeyDown(5, 4); // Y
            break;
            
        // Row 6: ENTER, L, K, J, H
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            ula->setKeyDown(6, 0); // ENTER
            break;
        case SDLK_L:
            ula->setKeyDown(6, 1); // L
            break;
        case SDLK_K:
            ula->setKeyDown(6, 2); // K
            break;
        case SDLK_J:
            ula->setKeyDown(6, 3); // J
            break;
        case SDLK_H:
            ula->setKeyDown(6, 4); // H
            break;
            
        // Row 7: SPACE, SYM SHIFT, M, N, B
        case SDLK_SPACE:
            ula->setKeyDown(7, 0); // SPACE
            break;
        case SDLK_RSHIFT:
            ula->setKeyDown(7, 1); // SYM SHIFT
            break;
        case SDLK_M:
            ula->setKeyDown(7, 2); // M
            break;
        case SDLK_N:
            ula->setKeyDown(7, 3); // N
            break;
        case SDLK_B:
            ula->setKeyDown(7, 4); // B
            break;
    }
}

// Handle key up events
void Emulator::handleKeyUp(SDL_Keycode key) {
    // Map SDL keys to ZX Spectrum keyboard matrix
    switch (key) {
        // Row 0: CAPS SHIFT, Z, X, C, V
        case SDLK_LSHIFT:
            ula->setKeyUp(0, 0); // CAPS SHIFT
            break;
        case SDLK_Z:
            ula->setKeyUp(0, 1); // Z
            break;
        case SDLK_X:
            ula->setKeyUp(0, 2); // X
            break;
        case SDLK_C:
            ula->setKeyUp(0, 3); // C
            break;
        case SDLK_V:
            ula->setKeyUp(0, 4); // V
            break;
            
        // Row 1: A, S, D, F, G
        case SDLK_A:
            ula->setKeyUp(1, 0); // A
            break;
        case SDLK_S:
            ula->setKeyUp(1, 1); // S
            break;
        case SDLK_D:
            ula->setKeyUp(1, 2); // D
            break;
        case SDLK_F:
            ula->setKeyUp(1, 3); // F
            break;
        case SDLK_G:
            ula->setKeyUp(1, 4); // G
            break;
            
        // Row 2: Q, W, E, R, T
        case SDLK_Q:
            ula->setKeyUp(2, 0); // Q
            break;
        case SDLK_W:
            ula->setKeyUp(2, 1); // W
            break;
        case SDLK_E:
            ula->setKeyUp(2, 2); // E
            break;
        case SDLK_R:
            ula->setKeyUp(2, 3); // R
            break;
        case SDLK_T:
            ula->setKeyUp(2, 4); // T
            break;
            
        // Row 3: 1, 2, 3, 4, 5
        case SDLK_1:
            ula->setKeyUp(3, 0); // 1
            break;
        case SDLK_2:
            ula->setKeyUp(3, 1); // 2
            break;
        case SDLK_3:
            ula->setKeyUp(3, 2); // 3
            break;
        case SDLK_4:
            ula->setKeyUp(3, 3); // 4
            break;
        case SDLK_5:
            ula->setKeyUp(3, 4); // 5
            break;
            
        // Row 4: 0, 9, 8, 7, 6
        case SDLK_0:
            ula->setKeyUp(4, 0); // 0
            break;
        case SDLK_9:
            ula->setKeyUp(4, 1); // 9
            break;
        case SDLK_8:
            ula->setKeyUp(4, 2); // 8
            break;
        case SDLK_7:
            ula->setKeyUp(4, 3); // 7
            break;
        case SDLK_6:
            ula->setKeyUp(4, 4); // 6
            break;
            
        // Row 5: P, O, I, U, Y
        case SDLK_P:
            ula->setKeyUp(5, 0); // P
            break;
        case SDLK_O:
            ula->setKeyUp(5, 1); // O
            break;
        case SDLK_I:
            ula->setKeyUp(5, 2); // I
            break;
        case SDLK_U:
            ula->setKeyUp(5, 3); // U
            break;
        case SDLK_Y:
            ula->setKeyUp(5, 4); // Y
            break;
            
        // Row 6: ENTER, L, K, J, H
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            ula->setKeyUp(6, 0); // ENTER
            break;
        case SDLK_L:
            ula->setKeyUp(6, 1); // L
            break;
        case SDLK_K:
            ula->setKeyUp(6, 2); // K
            break;
        case SDLK_J:
            ula->setKeyUp(6, 3); // J
            break;
        case SDLK_H:
            ula->setKeyUp(6, 4); // H
            break;
            
        // Row 7: SPACE, SYM SHIFT, M, N, B
        case SDLK_SPACE:
            ula->setKeyUp(7, 0); // SPACE
            break;
        case SDLK_RSHIFT:
            ula->setKeyUp(7, 1); // SYM SHIFT
            break;
        case SDLK_M:
            ula->setKeyUp(7, 2); // M
            break;
        case SDLK_N:
            ula->setKeyUp(7, 3); // N
            break;
        case SDLK_B:
            ula->setKeyUp(7, 4); // B
            break;
    }
}

int main(void)
{
    std::cout << "Starting emulator..." << std::endl;

    Emulator emulator;

    if (!emulator.initialize())
    {
        std::cerr << "Failed to initialize emulator!" << std::endl;
        return -1;
    }

    std::cout << "Emulator initialized successfully." << std::endl;
    std::cout << "Starting emulation loop..." << std::endl;

    emulator.run();

    std::cout << "Emulation loop ended." << std::endl;

    return 0;
}
