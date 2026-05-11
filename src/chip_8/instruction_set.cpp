#include "chip_8/instruction_set.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "chip_8/display.hpp"
#include "chip_8/error.hpp"
#include "chip_8/font.hpp"
#include "chip_8/keyboard.hpp"
#include "chip_8/utility.hpp"

namespace emu::instruction_set {

void op0nnn(ChipState& /* not used */, const std::uint16_t /* not used */) {}

void op00Cn(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleN = static_cast<std::size_t>(getNibbleN(bytecode));
    const auto kRowSize = display::kHighWidth;
    const auto kTotalRows = display::kHighHeight;

    std::memmove(state.display.buffer.data() + (kNibbleN * kRowSize),
                 state.display.buffer.data(),
                 (kTotalRows - kNibbleN) * kRowSize);

    std::memset(state.display.buffer.data(), 0x00, kNibbleN * kRowSize);
}

void op00E0(ChipState& state, const std::uint16_t /* not used */) {
    std::memset(state.display.buffer.data(), 0x00, state.display.buffer.size());
    state.display.draw = true;
}

void op00EE(ChipState& state, const std::uint16_t /* not used */) {
    state.program_counter = state.stack.pop();
}

void op00FB(ChipState& state, const std::uint16_t /* not used */) {
    constexpr std::size_t kShift = 4;
    const auto kRowSize = display::kHighWidth;

    // Shift right each row by 4 pixels
    for (std::size_t row = 0; row < display::kHighHeight; row++) {
        auto* row_ptr = state.display.buffer.data() + (row * kRowSize);

        std::memmove(row_ptr + kShift, row_ptr, kRowSize - kShift);

        std::memset(row_ptr, 0x00, kShift);
    }
}

void op00FC(ChipState& state, const std::uint16_t /* not used */) {
    constexpr std::size_t kShift = 4;
    const auto kRowSize = display::kHighWidth;

    // Shift left each row by 4 pixels
    for (std::size_t row = 0; row < display::kHighHeight; row++) {
        auto* row_ptr = state.display.buffer.data() + (row * kRowSize);

        std::memmove(row_ptr, row_ptr + kShift, kRowSize - kShift);

        std::memset(row_ptr + (kRowSize - kShift), 0x00, kShift);
    }
}

void op00FD(ChipState& state, const std::uint16_t /* not used */) {
    state.should_exit = true;
}

void op00FE(ChipState& state, const std::uint16_t /* not used */) {
    state.display.mode = display::Mode::kLow;
}

void op00FF(ChipState& state, const std::uint16_t /* not used */) {
    state.display.mode = display::Mode::kHigh;
}

void op1nnn(ChipState& state, const std::uint16_t bytecode) {
    state.program_counter = getAddress(bytecode);
}

void op2nnn(ChipState& state, const std::uint16_t bytecode) {
    state.stack.push(state.program_counter);
    state.program_counter = getAddress(bytecode);
}

void op3xkk(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] == getLowByte(bytecode)) {
        state.program_counter += 2;
    }
}

void op4xkk(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] != getLowByte(bytecode)) {
        state.program_counter += 2;
    }
}

void op5xy0(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] == state.V[getNibbleY(bytecode)]) {
        state.program_counter += 2;
    }
}

void op6xkk(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] = getLowByte(bytecode);
}

void op7xkk(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] += getLowByte(bytecode);
}

void op8xy0(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] = state.V[getNibbleY(bytecode)];
}

void op8xy1(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] |= state.V[getNibbleY(bytecode)];
}

void op8xy2(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] &= state.V[getNibbleY(bytecode)];
}

void op8xy3(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] ^= state.V[getNibbleY(bytecode)];
}

void op8xy4(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    const auto kResult =
        static_cast<unsigned int>(state.V[kNibbleX]) +
        static_cast<unsigned int>(state.V[getNibbleY(bytecode)]);

    state.V[0xF] = static_cast<std::uint8_t>((kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

void op8xy5(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    const auto kResult =
        static_cast<unsigned int>(state.V[kNibbleX]) -
        static_cast<unsigned int>(state.V[getNibbleY(bytecode)]);

    state.V[0xF] = static_cast<std::uint8_t>((~kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

void op8xy6(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Store vy into vx before shift
        state.V[kNibbleX] = state.V[getNibbleY(bytecode)];
    }

    const auto kTemp = static_cast<unsigned int>(state.V[kNibbleX]);
    state.V[0xF] = static_cast<std::uint8_t>(kTemp & 0x1U);
    state.V[kNibbleX] >>= 1U;
}

void op8xy7(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    const auto kResult =
        static_cast<unsigned int>(state.V[getNibbleY(bytecode)]) -
        static_cast<unsigned int>(state.V[kNibbleX]);

    state.V[0xF] = static_cast<std::uint8_t>((~kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

void op8xyE(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Store vy into vx before shift
        state.V[kNibbleX] = state.V[getNibbleY(bytecode)];
    }

    const auto kResult = static_cast<unsigned int>(state.V[kNibbleX]) << 1U;

    state.V[0xF] = static_cast<std::uint8_t>((kResult & 0x100U) >> kByteWidth);
    state.V[kNibbleX] = static_cast<std::uint8_t>(kResult);
}

void op9xy0(ChipState& state, const std::uint16_t bytecode) {
    if (state.V[getNibbleX(bytecode)] != state.V[getNibbleY(bytecode)]) {
        state.program_counter += 2U;
    }
}

void opAnnn(ChipState& state, const std::uint16_t bytecode) {
    state.index_register = getAddress(bytecode);
}

void opBnnn(ChipState& state, const std::uint16_t bytecode) {
    if (state.mode == Mode::kCosmacVIP) {
        // Original behavior: Add V0 to address
        state.program_counter = getAddress(bytecode) + state.V[0];
    } else {
        // Super behavior: Add Vx to address
        state.program_counter =
            getAddress(bytecode) + state.V[getNibbleX(bytecode)];
    }
}

void opCxkk(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] =
        static_cast<std::uint8_t>(state.rnd() & getAddress(bytecode));
}

void opDxyn(ChipState& state, const std::uint16_t bytecode) {
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

void opEx9E(ChipState& state, const std::uint16_t bytecode) {
    if (state.keyboard == nullptr) {
        throw NullKeyboardError("opEx9E");
    }

    const auto kKey = state.V[getNibbleX(bytecode)];

    if (state.keyboard[keyboard::mapping(kKey)]) {
        state.program_counter += 2U;
    }
}

void opExA1(ChipState& state, const std::uint16_t bytecode) {
    if (state.keyboard == nullptr) {
        throw NullKeyboardError("opExA1");
    }

    const auto kKey = state.V[getNibbleX(bytecode)];

    if (!state.keyboard[keyboard::mapping(kKey)]) {
        state.program_counter += 2U;
    }
}

void opFx07(ChipState& state, const std::uint16_t bytecode) {
    state.V[getNibbleX(bytecode)] = state.delay_timer;
}

void opFx0A(ChipState& state, const std::uint16_t bytecode) {
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

void opFx15(ChipState& state, const std::uint16_t bytecode) {
    state.delay_timer = state.V[getNibbleX(bytecode)];
}

void opFx18(ChipState& state, const std::uint16_t bytecode) {
    state.sound_timer = state.V[getNibbleX(bytecode)];
}

void opFx1E(ChipState& state, const std::uint16_t bytecode) {
    state.index_register += state.V[getNibbleX(bytecode)];
}

void opFx29(ChipState& state, const std::uint16_t bytecode) {
    const auto kDigit =
        static_cast<std::uint16_t>(state.V[getNibbleX(bytecode)]);

    state.index_register =
        font::kLowMemoryOffset + (kDigit * font::kLowSpriteSize);
}

void opFx30(ChipState& state, const std::uint16_t bytecode) {
    const auto kDigit =
        static_cast<std::uint16_t>(state.V[getNibbleX(bytecode)]);

    if (kDigit > 0x9U) {
        throw InvalidSpriteLoadError("opFx30");
    }

    state.index_register =
        font::kHighMemoryOffset + (kDigit * font::kHighSpriteSize);
}

void opFx33(ChipState& state, const std::uint16_t bytecode) {
    auto value = static_cast<unsigned int>(state.V[getNibbleX(bytecode)]);

    state.memory[state.index_register + 2] =
        static_cast<std::uint8_t>(value % 10U);
    value /= 10U;

    state.memory[state.index_register + 1] =
        static_cast<std::uint8_t>(value % 10U);
    value /= 10U;

    state.memory[state.index_register] = static_cast<std::uint8_t>(value % 10U);
}

void opFx55(ChipState& state, const std::uint16_t bytecode) {
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

void opFx65(ChipState& state, const std::uint16_t bytecode) {
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

void opFx75(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    for (std::uint8_t idx = 0; idx <= kNibbleX && idx < 8U; ++idx) {
        state.rpl[idx] = state.V[idx];
    }
}

void opFx85(ChipState& state, const std::uint16_t bytecode) {
    const auto kNibbleX = getNibbleX(bytecode);

    for (std::uint8_t idx = 0; idx <= kNibbleX && idx < 8U; ++idx) {
        state.V[idx] = state.rpl[idx];
    }
}

}  // namespace emu::instruction_set