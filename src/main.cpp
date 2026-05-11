#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "chip_8/chip_8.hpp"

#include "SDL3/SDL_log.h"
#include "cxxopts.hpp"

int main(int argc, char* argv[]) {
    cxxopts::Options options("chip-8", "A simple CHIP-8 emulator");

    auto option_adder = options.add_options();
    option_adder("r,rom", "ROM file path",
                 cxxopts::value<std::filesystem::path>()->default_value(
                     "roms/snake.ch8"));
    option_adder("s,speed", "Instruction speed in Hz",
                 cxxopts::value<int>()->default_value("700"));
    option_adder("z,scale", "Window scale factor",
                 cxxopts::value<float>()->default_value("10.0"));
    option_adder("f,fullscreen", "Enable fullscreen mode",
                 cxxopts::value<bool>()->default_value("false"));
    option_adder("m,mode", "Emulation mode",
                 cxxopts::value<std::string>()->default_value("super_chip"));
    option_adder("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.contains("help")) {
        std::cout << options.help() << '\n';
        return 0;
    }

    const auto kRomPath = result["rom"].as<std::filesystem::path>();
    const auto kSpeedHz = result["speed"].as<int>();
    const auto kScale = result["scale"].as<float>();
    const auto kFullscreen = result["fullscreen"].as<bool>();
    const auto kMode = emu::stringToMode(result["mode"].as<std::string>());
    try {
        emu::Chip8 interpreter(kMode, kSpeedHz, kFullscreen, kScale);

        interpreter.load(kRomPath);

        interpreter.start();
    } catch (const std::exception& error) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "An error occurred: %s",
                     error.what());
        return 1;
    }

    return 0;
}
