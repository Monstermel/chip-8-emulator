#ifndef CHIP_8_CHIP_STATE_HPP
#define CHIP_8_CHIP_STATE_HPP

#include <atomic>
#include <cstdint>
#include <random>
#include <stack>

#include "chip_8/display.hpp"
#include "chip_8/keyboard.hpp"
#include "chip_8/memory.hpp"
#include "chip_8/registers.hpp"

namespace emu {

namespace font {  // Font metadata
constexpr std::uint16_t kSpriteSize = 5;
constexpr std::uint16_t kMemoryOffset = 0x000;
}  // namespace font

struct ChipState {
    // Memory
    memory::Type memory{
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
    };
    // Display buffer
    display::Type display;
    // Registers
    registers::Type V{};
    // Random engine
    std::minstd_rand rnd;
    // Program counter
    std::uint16_t program_counter{memory::kProgramSpaceOffset};
    // Index register
    std::uint16_t index_register{};
    // Delay Timer
    std::atomic<std::uint8_t> delay_timer{0};
    // Sound Timer
    std::atomic<std::uint8_t> sound_timer{0};

    // Keyboard state
    keyboard::Type keyboard{};

    // Stack TODO: Implement a static stack
    std::stack<std::uint16_t> stack;
};

}  // namespace emu

#endif /* CHIP_8_CHIP_STATE_HPP */
