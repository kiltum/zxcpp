#include <SDL3/SDL.h>
#include <iostream>
#include <memory>
#include "ula.hpp"
#include "memory.hpp"
#include "port.hpp"
#include "z80.hpp"

class Emulator {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    bool quit;
    
    // Emulator components
    std::unique_ptr<Memory> memory;
    std::unique_ptr<Port> ports;
    std::unique_ptr<Z80> cpu;
    std::unique_ptr<ULA> ula;
    
public:
    Emulator() : window(nullptr), renderer(nullptr), texture(nullptr), quit(false) {}
    
    ~Emulator() {
        cleanup();
    }
    
    bool initialize() {
        // Initialize emulator components
        memory = std::make_unique<Memory>();
        ports = std::make_unique<Port>();
        cpu = std::make_unique<Z80>(memory.get(), ports.get());
        ula = std::make_unique<ULA>(memory.get());
        
        // Register ULA with port system
        ports->RegisterReadHandler(0xFE, [this](uint16_t port) -> uint8_t {
            return ula->readPort(port);
        });
        
        ports->RegisterWriteHandler(0xFE, [this](uint16_t port, uint8_t value) {
            ula->writePort(port, value);
        });
        
        // Initialize SDL
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            std::cerr << "SDL Error code: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create window
        window = SDL_CreateWindow("ZX Spectrum Emulator", 640, 480, 0);
        if (window == nullptr) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Show window
        SDL_ShowWindow(window);
        
        // Create renderer
        renderer = SDL_CreateRenderer(window, nullptr);
        if (renderer == nullptr) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create texture for the screen (352x288 to match ULA buffer)
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                   SDL_TEXTUREACCESS_STATIC, 352, 288);
        if (texture == nullptr) {
            std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        
        return true;
    }
    
    void run() {
        SDL_Event e;
        
        std::cout << "Entering emulation loop" << std::endl;
        
        while (!quit) {
            // Handle events
            while (SDL_PollEvent(&e) != 0) {
                std::cout << "Event: " << e.type << std::endl;
                if (e.type == SDL_EVENT_QUIT) {
                    std::cout << "Quit event received" << std::endl;
                    quit = true;
                }
            }
            
            // Process ULA ticks (approximately 3.5MHz)
            for (int i = 0; i < 80000; i++) {
                ula->oneTick(0);
            }
            
            // Clear screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            
            // Update screen with ULA output
            uint32_t* src = ula->getScreenBuffer();
            
            // Update texture with ULA screen buffer
            SDL_UpdateTexture(texture, nullptr, src, 352 * sizeof(uint32_t));
            
            // Render texture
            SDL_RenderTexture(renderer, texture, nullptr, nullptr);
            
            // Update screen
            SDL_RenderPresent(renderer);
            
            // Add a small delay to slow down the loop
            SDL_Delay(16); // Approximately 60 FPS
        }
    }
    
    void cleanup() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        
        SDL_Quit();
    }
};

int main(void) {
    std::cout << "Starting emulator..." << std::endl;
    
    Emulator emulator;
    
    if (!emulator.initialize()) {
        std::cerr << "Failed to initialize emulator!" << std::endl;
        return -1;
    }
    
    std::cout << "Emulator initialized successfully." << std::endl;
    std::cout << "Starting emulation loop..." << std::endl;
    
    emulator.run();
    
    std::cout << "Emulation loop ended." << std::endl;
    
    return 0;
}
