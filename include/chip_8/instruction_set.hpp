#ifndef CHIP_8_INSTRUCTION_SET_HPP
#define CHIP_8_INSTRUCTION_SET_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "chip_8/chip_state.hpp"
#include "chip_8/display.hpp"
#include "chip_8/error.hpp"
#include "chip_8/font.hpp"
#include "chip_8/keyboard.hpp"
#include "chip_8/utility.hpp"

namespace emu::instruction_set {

/**
 * @brief JMP to a host machine code - Treated as NOP
 *
 * @param bytecode
 */
inline void op0nnn(ChipState& /* not used */,
                   const std::uint16_t /* not used */) {}

/**
 * @brief SCD nibble - Scroll display down n rows.
 *
 * @param bytecode
 */
inline void op00Cn(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleN = static_cast<std::size_t>(getNibbleN(bytecode));
    const auto kRowSize = display::kHighWidth;
    const auto kTotalRows = display::kHighHeight;

    std::memmove(state.display.buffer.data() + (kNibbleN * kRowSize),
                 state.display.buffer.data(),
                 (kTotalRows - kNibbleN) * kRowSize);

    std::memset(state.display.buffer.data(), 0x00, kNibbleN * kRowSize);
}

/**
 * @brief CLS - Clear the display.
 *
 * @param bytecode
 */
inline void op00E0(ChipState& state, const std::uint16_t /* not used */) {
    std::memset(state.display.buffer.data(), 0x00, state.display.buffer.size());
    state.display.draw = true;
}

/**
 * @brief Return Call - The interpreter sets the program counter to the
 * address at the top of the stack, then subtracts 1 from the stack pointer.
 *
 * @param bytecode
 */
inline void op00EE(ChipState& state, const std::uint16_t /* not used */) {
    state.program_counter = state.stack.pop();
}

/**
 * @brief SCR - Scroll display right.
 *
 * @param bytecode
 */
inline void op00FB(ChipState& state, const std::uint16_t /* not used */) {
    constexpr std::size_t kShift = 4;
    const auto kRowSize = display::kHighWidth;

    // Shift right each row by 4 pixels
    for (std::size_t row = 0; row < display::kHighHeight; row++) {
        auto* row_ptr = state.display.buffer.data() + (row * kRowSize);

        std::memmove(row_ptr + kShift, row_ptr, kRowSize - kShift);

        std::memset(row_ptr, 0x00, kShift);
    }
}

/**
 * @brief SCL - Scroll display left.
 *
 * @param bytecode
 */
inline void op00FC(ChipState& state, const std::uint16_t /* not used */) {
    constexpr std::size_t kShift = 4;
    const auto kRowSize = display::kHighWidth;

    // Shift left each row by 4 pixels
    for (std::size_t row = 0; row < display::kHighHeight; row++) {
        auto* row_ptr = state.display.buffer.data() + (row * kRowSize);

        std::memmove(row_ptr, row_ptr + kShift, kRowSize - kShift);

        std::memset(row_ptr + (kRowSize - kShift), 0x00, kShift);
    }
}

/**
 * @brief Exit - Exit interpreter.
 *
 * @param bytecode
 */
inline void op00FD(ChipState& state, const std::uint16_t /* not used */) {
    state.should_exit = true;
}

/**
 * @brief Low - Set low resolution.
 *
 * @param bytecode
 */
inline void op00FE(ChipState& state, const std::uint16_t /* not used */) {
    state.display.mode = display::Mode::kLow;
}

/**
 * @brief High - Set high resolution.
 *
 * @param bytecode
 */
inline void op00FF(ChipState& state, const std::uint16_t /* not used */) {
    state.display.mode = display::Mode::kHigh;
}

/**
 * @brief JMP to address - The interpreter sets the program counter to nnn.
 *
 * @param bytecode
 */
inline void op1nnn(ChipState& state, const std::uint16_t bytecode) {
    state.program_counter = getAddress(bytecode);
}

/**
 * @brief Call address - The interpreter increments the stack pointer, then
 * puts the current PC on the top of the stack. The PC is then set to nnn.
 *
 * @param bytecode
 */
inline void op2nnn(ChipState& state, const std::uint16_t bytecode) {
    state.stack.push(state.program_counter);
    state.program_counter = getAddress(bytecode);
}

/**
 * @brief SE Vx, byte - Skip next bytecode if Vx = kk.
 *
 * @param bytecode
 */
inline void op3xkk(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] == getLowByte(bytecode)) {
        state.program_counter += 2;
    }
}

/**
 * @brief SNE Vx, byte - Skip next bytecode if Vx != kk.
 *
 * @param bytecode
 */
inline void op4xkk(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] != getLowByte(bytecode)) {
        state.program_counter += 2;
    }
}

/**
 * @brief SE Vx, Vy - Skip next bytecode if Vx = Vy.
 *
 * @param bytecode
 */
inline void op5xy0(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] == state.V[getNibbleY(bytecode)]) {
        state.program_counter += 2;
    }
}

/**
 * @brief LD Vx, byte - Set Vx = kk.
 *
 * @param bytecode
 */
inline void op6xkk(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] = getLowByte(bytecode);
}

/**
 * @brief ADD Vx, byte - Set Vx = Vx + kk.
 *
 * @param bytecode
 */
inline void op7xkk(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] += getLowByte(bytecode);
}

/**
 * @brief LD Vx, Vy - Set Vx = Vy.
 *
 * @param bytecode
 */
inline void op8xy0(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] = state.V[getNibbleY(bytecode)];
}

/**
 * @brief OR Vx, Vy - Set Vx = Vx OR Vy.
 *
 * @param bytecode
 */
inline void op8xy1(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] |= state.V[getNibbleY(bytecode)];
}

/**
 * @brief AND Vx, Vy - Set Vx = Vx AND Vy.
 *
 * @param bytecode
 */
inline void op8xy2(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] &= state.V[getNibbleY(bytecode)];
}

/**
 * @brief XOR Vx, Vy - Set Vx = Vx XOR Vy.
 *
 * @param bytecode
 */
inline void op8xy3(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] ^= state.V[getNibbleY(bytecode)];
}

/**
 * @brief ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry.
 *
 * @param bytecode
 */
inline void op8xy4(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    const auto kResult =
        static_cast<unsigned int>(state.V[kNibbleX]) +
        static_cast<unsigned int>(state.V[getNibbleY(bytecode)]);

    state.V[0xF] = static_cast<std::uint8_t>((kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

/**
 * @brief SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow.
 *
 * @param bytecode
 */
inline void op8xy5(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    const auto kResult =
        static_cast<unsigned int>(state.V[kNibbleX]) -
        static_cast<unsigned int>(state.V[getNibbleY(bytecode)]);

    state.V[0xF] = static_cast<std::uint8_t>((~kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

/**
 * @brief SHR Vx {, Vy} - Set Vx = Vx SHR 1.
 *
 * @param bytecode
 */
inline void op8xy6(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Store vy into vx before shift
        state.V[kNibbleX] = state.V[getNibbleY(bytecode)];
    }

    const auto kTemp = static_cast<unsigned int>(state.V[kNibbleX]);
    state.V[0xF] = static_cast<std::uint8_t>(kTemp & 0x1U);
    state.V[kNibbleX] >>= 1U;
}

/**
 * @brief SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow.
 *
 * @param bytecode
 */
inline void op8xy7(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    const auto kResult =
        static_cast<unsigned int>(state.V[getNibbleY(bytecode)]) -
        static_cast<unsigned int>(state.V[kNibbleX]);

    state.V[0xF] = static_cast<std::uint8_t>((~kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

/**
 * @brief SHL Vx {, Vy} - Set Vx = Vx SHL 1.
 *
 * @param bytecode
 */
inline void op8xyE(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Store vy into vx before shift
        state.V[kNibbleX] = state.V[getNibbleY(bytecode)];
    }

    const auto kResult = static_cast<unsigned int>(state.V[kNibbleX]) << 1U;

    state.V[0xF] = static_cast<std::uint8_t>((kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

/**
 * @brief SNE Vx, Vy - Skip next bytecode if Vx != Vy.
 *
 * @param bytecode
 */
inline void op9xy0(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] != state.V[getNibbleY(bytecode)]) {
        state.program_counter += 2U;
    }
}

/**
 * @brief LD I, addr - Set I = nnn.
 *
 * @param bytecode
 */
inline void opAnnn(ChipState& state, const std::uint16_t bytecode) {
    state.index_register = getAddress(bytecode);
}

/**
 * @brief JP V0, addr - Jump to location nnn + V0.
 * @note COSMAC VIP behavior. Use V0 instead of Vx.
 * @param bytecode
 */
inline void opBnnn(ChipState& state, const std::uint16_t bytecode) {
    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Add V0 to address
        state.program_counter = getAddress(bytecode) + state.V[0];
    } else {
        // Super behavior: Add Vx to address
        state.program_counter =
            getAddress(bytecode) + state.V[getNibbleX(bytecode)];
    }
}

/**
 * @brief RND Vx, byte - Set Vx = random byte AND kk.
 *
 * @param bytecode
 */
inline void opCxkk(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] =
        static_cast<std::uint8_t>(state.rnd() & getAddress(bytecode));
}

/**
 * @brief DRW Vx, Vy, nibble - Display n-byte sprite starting at memory
 * location I at (Vx, Vy), set VF = collision.
 *
 * @param bytecode
 */
inline void opDxyn(ChipState& state, const std::uint16_t bytecode) {
    const std::size_t kWidth = (state.display.mode == display::Mode::kLow)
                                   ? display::kLowWidth
                                   : display::kHighWidth;
    const std::size_t kHeight = (state.display.mode == display::Mode::kLow)
                                    ? display::kLowHeight
                                    : display::kHighHeight;

    const auto kCordX =
        static_cast<std::size_t>(state.V[getNibbleX(bytecode)]) % kWidth;
    const auto kCordY =
        static_cast<std::size_t>(state.V[getNibbleY(bytecode)]) % kHeight;

    const auto kNibbleN = static_cast<std::size_t>(getNibbleN(bytecode));
    const auto kSpriteSize = (kNibbleN > 0) ? kNibbleN : 16;

    state.V[0xF] = 0U;
    for (std::size_t j = 0; j < kSpriteSize; j++) {
        if ((kCordY + j) == kHeight) {
            break;
        }

        const auto kSprite =
            static_cast<unsigned int>(state.memory[state.index_register + j]);
        for (std::size_t i = 0; i < kByteWidth; i++) {
            if ((kCordX + i) == kWidth) {
                break;
            }

            auto& old_pixel =
                state.display.buffer[(kCordX + i) + ((kCordY + j) * kWidth)];
            const auto kNewPixel = (kSprite >> (kByteWidth - (i + 1U))) & 0x1U;

            state.V[0xF] |= static_cast<std::uint8_t>(kNewPixel & old_pixel);
            old_pixel ^= static_cast<std::uint8_t>(kNewPixel);
        }
    }

    state.display.draw = true;
}

/**
 * @brief SKP Vx - Skip next bytecode if key with the value of Vx is
 * pressed.
 *
 * @param bytecode
 */
inline void opEx9E(ChipState& state, const std::uint16_t bytecode) {
    if (state.keyboard == nullptr) {
        throw NullKeyboardError("opEx9E");
    }

    const auto kKey = state.V[getNibbleX(bytecode)];

    if (state.keyboard[keyboard::mapping(kKey)]) {
        state.program_counter += 2U;
    }
}

/**
 * @brief SKNP Vx - Skip next bytecode if key with the value of Vx is not
 * pressed.
 *
 * @param bytecode
 */
inline void opExA1(ChipState& state, const std::uint16_t bytecode) {
    if (state.keyboard == nullptr) {
        throw NullKeyboardError("opExA1");
    }

    const auto kKey = state.V[getNibbleX(bytecode)];

    if (!state.keyboard[keyboard::mapping(kKey)]) {
        state.program_counter += 2U;
    }
}

/**
 * @brief LD Vx, DT - Set Vx = delay timer value.
 *
 * @param bytecode
 */
inline void opFx07(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] = state.delay_timer;
}

/**
 * @brief LD Vx, K - Wait for a key press, store the value of the key in Vx.
 *
 * @param bytecode
 */
inline void opFx0A(ChipState& state, const std::uint16_t bytecode) {
    if (state.keyboard == nullptr) {
        throw NullKeyboardError("opFx0A");
    }

    int key_pressed = -1;
    for (std::uint8_t key = 0;
         key < static_cast<std::uint8_t>(keyboard::kNumKeys); key++) {
        if (state.keyboard[keyboard::mapping(key)]) {
            key_pressed = key;
            break;
        }
    }

    if (key_pressed == -1) {
        // Run this instruction again in the next cycle
        state.program_counter -= 2U;
    } else {
        state.V[getNibbleX(bytecode)] = static_cast<std::uint8_t>(key_pressed);
    }
}

/**
 * @brief LD DT, Vx - Set delay timer = Vx.
 *
 * @param bytecode
 */
inline void opFx15(ChipState& state, const std::uint16_t bytecode) {
    state.delay_timer = state.V[getNibbleX(bytecode)];
}

/**
 * @brief LD ST, Vx - Set sound timer = Vx.
 *
 * @param bytecode
 */
inline void opFx18(ChipState& state, const std::uint16_t bytecode) {
    state.sound_timer = state.V[getNibbleX(bytecode)];
}

/**
 * @brief ADD I, Vx - Set I = I + Vx.
 *
 * @param bytecode
 */
inline void opFx1E(ChipState& state, const std::uint16_t bytecode) {
    state.index_register += state.V[getNibbleX(bytecode)];
}

/**
 * @brief LD F, Vx - Set I = location of sprite for digit Vx.
 *
 * @param bytecode
 */
inline void opFx29(ChipState& state, const std::uint16_t bytecode) {
    const auto kDigit =
        static_cast<std::uint16_t>(state.V[getNibbleX(bytecode)]);

    state.index_register =
        font::kLowMemoryOffset + (kDigit * font::kLowSpriteSize);
}

/**
 * @brief LD HF, Vx - Set I = location of 16x16 sprite for digit Vx.
 *
 * @param bytecode
 */
inline void opFx30(ChipState& state, const std::uint16_t bytecode) {
    const auto kDigit =
        static_cast<std::uint16_t>(state.V[getNibbleX(bytecode)]);

    if (kDigit > 0x9U) {
        throw InvalidSpriteLoadError("opFx30");
    }

    state.index_register =
        font::kHighMemoryOffset + (kDigit * font::kHighSpriteSize);
}

/**
 * @brief LD B, Vx - Store BCD representation of Vx in memory locations I,
 * I+1, and I+2.
 *
 * @param bytecode
 */
inline void opFx33(ChipState& state, const std::uint16_t bytecode) {
    auto value = static_cast<unsigned int>(state.V[getNibbleX(bytecode)]);

    state.memory[state.index_register + 2] =
        static_cast<std::uint8_t>(value % 10U);
    value /= 10U;

    state.memory[state.index_register + 1] =
        static_cast<std::uint8_t>(value % 10U);
    value /= 10U;

    state.memory[state.index_register] = static_cast<std::uint8_t>(value % 10U);
}

/**
 * @brief LD [I], Vx - Store registers V0 through Vx in memory starting at
 * location I.
 *
 * @param bytecode
 */
inline void opFx55(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Update [I] after each store
        for (int rgs = 0; rgs <= kNibbleX; rgs++) {
            state.memory[state.index_register] = state.V[rgs];
            state.index_register++;
        }
    } else {
        // Super behavior: Don't update [I] after each store
        for (int idx = state.index_register, rgs = 0; rgs <= kNibbleX;
             idx++, rgs++) {
            state.memory[idx] = state.V[rgs];
        }
    }
}

/**
 * @brief Fx65 - LD Vx, [I] - Load memory starting at location I into registers
 * V0 through Vx.
 *
 * @param bytecode
 */
inline void opFx65(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Update [I] after each load
        for (int rgs = 0; rgs <= kNibbleX; rgs++) {
            state.V[rgs] = state.memory[state.index_register];
            state.index_register++;
        }
    } else {
        // Super behavior: Don't update [I] after each load
        for (int idx = state.index_register, rgs = 0; rgs <= kNibbleX;
             idx++, rgs++) {
            state.V[rgs] = state.memory[idx];
        }
    }
}

/**
 * @brief LD R, Vx - Store registers V0 through Vx in RPL user flags
 * (X=0..7).
 *
 * @param bytecode
 */
inline void opFx75(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    for (std::uint8_t idx = 0; idx <= kNibbleX && idx < 8U; ++idx) {
        state.rpl.flags[idx] = state.V[idx];
    }
    state.rpl.dirty = true;
}

/**
 * @brief LD Vx, R - Load RPL user flags (X=0..7) into registers V0 through
 * Vx.
 *
 * @param bytecode
 */
inline void opFx85(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    for (std::uint8_t idx = 0; idx <= kNibbleX && idx < 8U; ++idx) {
        state.V[idx] = state.rpl.flags[idx];
    }
}

}  // namespace emu::instruction_set

#endif /* CHIP_8_INSTRUCTION_SET_HPP */
