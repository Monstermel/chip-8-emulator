#ifndef CHIP_8_CHIP_8_HPP
#define CHIP_8_CHIP_8_HPP

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>

#include "chip_8/chip_state.hpp"
#include "chip_8/display.hpp"
#include "chip_8/error.hpp"
#include "chip_8/frontend.hpp"
#include "chip_8/instruction_set.hpp"
#include "chip_8/utility.hpp"

#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace emu {

class Chip8 {
    ChipState state_;
    Frontend frontend_;
    std::jthread timer_thread_;
    std::chrono::nanoseconds accumulator_{0};

    // RODO: Move most of it into a backend class so Chip8 act only as a facade

    /**
     * @brief Fetch an instruction from memory and update program counter
     *
     * @return instruction in bytecode format
     */
    std::uint16_t fetch() noexcept {
        const auto kInstruction =
            (static_cast<unsigned int>(state_.memory[state_.program_counter])
             << kByteWidth) |
            static_cast<unsigned int>(
                state_.memory[state_.program_counter + 1]);

        state_.program_counter += 2;

        return static_cast<std::uint16_t>(kInstruction);
    }

    /**
     * @brief Map bytecode to its instruction under 0xxx group
     *
     * @param bytecode
     * @return instruction_set::Instruction
     */
    static instruction_set::Instruction handleGroup0(
        const std::uint16_t bytecode) {
        switch (bytecode & 0x0FFFU) {
            case 0x00E0:
                return instruction_set::op00E0;
            case 0x00EE:
                return instruction_set::op00EE;
            default:
                return instruction_set::op0nnn;
        }
    }

    /**
     * @brief Map bytecode to its instruction under 8xxx group
     *
     * @param bytecode
     * @return instruction_set::Instruction
     */
    static instruction_set::Instruction handleGroup8(
        const std::uint16_t bytecode) {
        switch (bytecode & 0x000FU) {
            case 0x0000:
                return instruction_set::op8xy0;
            case 0x0001:
                return instruction_set::op8xy1;
            case 0x0002:
                return instruction_set::op8xy2;
            case 0x0003:
                return instruction_set::op8xy3;
            case 0x0004:
                return instruction_set::op8xy4;
            case 0x0005:
                return instruction_set::op8xy5;
            case 0x0006:
                return instruction_set::op8xy6;
            case 0x0007:
                return instruction_set::op8xy7;
            case 0x000E:
                return instruction_set::op8xyE;
            default:
                throw InvalidInstructionError(bytecode);
        }
    }

    /**
     * @brief Map bytecode to its instruction under Exxx group
     *
     * @param bytecode
     * @return instruction_set::Instruction
     */
    static instruction_set::Instruction handleGroupE(
        const std::uint16_t bytecode) {
        switch (bytecode & 0x00FFU) {
            case 0x009E:
                return instruction_set::opEx9E;
            case 0x00A1:
                return instruction_set::opExA1;
            default:
                throw InvalidInstructionError(bytecode);
        }
    }

    /**
     * @brief Map bytecode to its instruction under Fxxx group
     *
     * @param bytecode
     * @return instruction_set::Instruction
     */
    static instruction_set::Instruction handleGroupF(
        const std::uint16_t bytecode) {
        switch (bytecode & 0x00FFU) {
            case 0x0007:
                return instruction_set::opFx07;
            case 0x000A:
                return instruction_set::opFx0A;
            case 0x0015:
                return instruction_set::opFx15;
            case 0x0018:
                return instruction_set::opFx18;
            case 0x001E:
                return instruction_set::opFx1E;
            case 0x0029:
                return instruction_set::opFx29;
            case 0x0033:
                return instruction_set::opFx33;
            case 0x0055:
                return instruction_set::opFx55;
            case 0x0065:
                return instruction_set::opFx65;
            default:
                throw InvalidInstructionError(bytecode);
        }
    }

    /**
     * @brief Map a bytecode to its corresponding instruction
     *
     * @param instruction
     * @return instruction_set::Instruction
     */
    static instruction_set::Instruction decode(const std::uint16_t bytecode) {
        switch (bytecode & 0xF000U) {
            case 0x0000:
                return handleGroup0(bytecode);
            case 0x1000:
                return instruction_set::op1nnn;
            case 0x2000:
                return instruction_set::op2nnn;
            case 0x3000:
                return instruction_set::op3xkk;
            case 0x4000:
                return instruction_set::op4xkk;
            case 0x5000:
                return instruction_set::op5xy0;
            case 0x6000:
                return instruction_set::op6xkk;
            case 0x7000:
                return instruction_set::op7xkk;
            case 0x8000:
                return handleGroup8(bytecode);
            case 0x9000:
                return instruction_set::op9xy0;
            case 0xA000:
                return instruction_set::opAnnn;
            case 0xB000:
                return instruction_set::opBnnn;
            case 0xC000:
                return instruction_set::opCxkk;
            case 0xD000:
                return instruction_set::opDxyn;
            case 0xE000:
                return handleGroupE(bytecode);
            case 0xF000:
                return handleGroupF(bytecode);
            default:
                throw InvalidInstructionError(bytecode);
        }
    }

   public:
    Chip8()
        : timer_thread_([this](const std::stop_token& token) {
              while (!token.stop_requested()) {
                  if (state_.delay_timer > 0) {
                      state_.delay_timer -= 1;
                  }
                  frontend_.handleSound(state_.sound_timer);

                  if (state_.display.draw) {
                      frontend_.renderDisplay(state_.display);
                      state_.display.draw = false;
                  }

                  using namespace std::chrono_literals;
                  std::this_thread::sleep_for(16666us);  // ~60 Hz
              }
          }) {}

    /**
     * @brief Load test ROM into memory
     *
     * @return 0 at success, -1 at failure
     */
    int load();

    /**
     * @brief Represet a single interpreter cycle
     *
     */
    void cycle(std::chrono::nanoseconds dt) {
        // 1. Accumulate time for instructions (700Hz = ~1,428,571 ns per
        // instruction)
        accumulator_ += dt;
        while (accumulator_ >= std::chrono::nanoseconds(1428571)) {
            accumulator_ -= std::chrono::nanoseconds(1428571);

            state_.keyboard = SDL_GetKeyboardState(NULL);

            const auto kBytecode = fetch();

            const auto kInstruction = decode(kBytecode);

            kInstruction(state_, kBytecode);
        }
    }
};

}  // namespace emu

#endif /* CHIP_8_CHIP_8_HPP */
