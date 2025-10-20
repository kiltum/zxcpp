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
