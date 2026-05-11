#ifndef CHIP_8_CHIP_STATE_HPP
#define CHIP_8_CHIP_STATE_HPP

#include <cstdint>
#include <random>

#include "chip_8/display.hpp"
#include "chip_8/font.hpp"
#include "chip_8/keyboard.hpp"
#include "chip_8/memory.hpp"
#include "chip_8/registers.hpp"
#include "chip_8/rpl_flags.hpp"
#include "chip_8/stack.hpp"

namespace emu {

enum class Mode : std::uint8_t { kCosmacVIP, kSuperChip };

inline Mode stringToMode(const std::string& mode_str) {
    if (mode_str == "cosmac_vip") {
        return Mode::kCosmacVIP;
    }
    if (mode_str == "super_chip") {
        return Mode::kSuperChip;
    }

    throw std::invalid_argument("Invalid mode: " + mode_str);
}

struct ChipState {
    // Display buffer
    display::Type display;
    // Memory
    memory::Type memory{font::loadData()};
    // Stack
    stack::Type stack;
    // Registers
    registers::Type V{};
    // RPL Flags
    rpl_flags::Type rpl{};
    // Keyboard state
    keyboard::Type keyboard{};
    // Random engine
    std::minstd_rand rnd{std::random_device{}()};
    // Program counter
    std::uint16_t program_counter{memory::kProgramSpaceOffset};
    // Index register
    std::uint16_t index_register{};
    // Delay Timer
    std::uint8_t delay_timer{0};
    // Sound Timer
    std::uint8_t sound_timer{0};
    // Mode (Standard or Super)
    Mode mode{Mode::kSuperChip};
    // Should exit
    bool should_exit{false};
};

}  // namespace emu

#endif /* CHIP_8_CHIP_STATE_HPP */
