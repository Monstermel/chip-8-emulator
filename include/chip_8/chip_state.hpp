#ifndef CHIP_8_CHIP_STATE_HPP
#define CHIP_8_CHIP_STATE_HPP

#include <array>
#include <cstdint>
#include <random>

#include "chip_8/display.hpp"
#include "chip_8/font.hpp"
#include "chip_8/keyboard.hpp"
#include "chip_8/memory.hpp"
#include "chip_8/registers.hpp"
#include "chip_8/rpl_flags.hpp"

namespace emu {

struct ChipState {
    // Memory (4K)
    memory::Type memory{font::loadData()};
    // Display buffer
    display::Type display;
    // Registers
    registers::Type V{};
    // RPL Flags
    rpl_flags::Type rpl{};
    // Random engine
    std::minstd_rand rnd;
    // Program counter
    std::uint16_t program_counter{memory::kProgramSpaceOffset};
    // Index register
    std::uint16_t index_register{};
    // Delay Timer
    std::uint8_t delay_timer{0};
    // Sound Timer
    std::uint8_t sound_timer{0};

    // Keyboard state
    keyboard::Type keyboard{};

    // Stack
    static constexpr std::size_t kStackSize = 16;
    std::array<std::uint16_t, kStackSize> stack{};
    std::uint8_t stack_pointer{0};

    // Should exit
    bool should_exit{false};
};

}  // namespace emu

#endif /* CHIP_8_CHIP_STATE_HPP */
