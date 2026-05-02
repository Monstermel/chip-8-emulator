#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "chip_8/chip_8.hpp"

#include "cxxopts.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

int main(int argc, char* argv[]) {
    cxxopts::Options options("chip-8", "A simple CHIP-8 emulator");

    options.add_options()("r,rom", "ROM file path",
                          cxxopts::value<std::string>())(
        "h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (!result.count("rom")) {
        std::cerr << "Error: ROM file path is required. Use --rom <path>"
                  << std::endl;
        return 1;
    }

    std::string rom_path = result["rom"].as<std::string>();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to init sdl: %s",
                     SDL_GetError());
        return 1;
    }

    try {
        auto interpreter = std::make_unique<emu::Chip8>();
        if (interpreter->load(rom_path) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to load ROM: %s",
                         rom_path.c_str());
            SDL_Quit();
            return 1;
        }

        bool running = true;
        auto last_time = std::chrono::steady_clock::now();

        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
            }

            auto now = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now - last_time);
            last_time = now;

            try {
                interpreter->cycle(delta);
            } catch (const std::exception& error) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Chip8::cycle failed: %s", error.what());
                running = false;
            }
        }
    } catch (const std::exception& error) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "An error occurred: %s",
                     error.what());
        SDL_Quit();
        return 1;
    }

    SDL_Quit();
    return 0;
}
