#ifndef TEST_INTRUCTION_SET_HPP
#define TEST_INTRUCTION_SET_HPP

#include <algorithm>
#include <memory>

#include "chip_8/chip_state.hpp"
#include "chip_8/error.hpp"
#include "chip_8/instruction_set.hpp"
#include "chip_8/keyboard.hpp"

#include "SDL3/SDL_scancode.h"
#include "gtest/gtest.h"

namespace emu::instruction_set::test {

class Chip8OpcodeTest : public ::testing::Test {
   protected:
    std::unique_ptr<emu::ChipState> state_;

    void SetUp() override {
        // Reset state between tests
        state_ = std::make_unique<emu::ChipState>();
    }
};

// ============================================================================
// System Instructions (0nnn, 00E0, 00EE)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op0nnn_DoesNothing) {
    emu::instruction_set::op0nnn(*state_, 0x0123);
    EXPECT_EQ(state_->program_counter, emu::memory::kProgramSpaceOffset);
}

TEST_F(Chip8OpcodeTest, Op00E0_ClearsDisplay) {
    // Fill display buffer with non-zero values
    std::ranges::fill(state_->display.buffer.begin(),
                      state_->display.buffer.end(), 0xFF);
    state_->display.draw = false;

    emu::instruction_set::op00E0(*state_, 0x00E0);

    // Verify all pixels are cleared
    EXPECT_TRUE(std::ranges::all_of(
        state_->display.buffer.cbegin(), state_->display.buffer.cend(),
        [](std::uint8_t var) { return var == 0x00; }));
    EXPECT_TRUE(state_->display.draw);
}

TEST_F(Chip8OpcodeTest, Op00EE_ReturnsFromSubroutine) {
    state_->stack.push(0x300);
    state_->program_counter = 0x400;

    emu::instruction_set::op00EE(*state_, 0x00EE);

    EXPECT_EQ(state_->program_counter, 0x300);
    EXPECT_TRUE(state_->stack.empty());
}

TEST_F(Chip8OpcodeTest, Op00EE_ThrowsOnEmptyStack) {
    EXPECT_THROW(emu::instruction_set::op00EE(*state_, 0x00EE),
                 emu::StackUnderflowError);
}

// ============================================================================
// Jump Instructions (1nnn, Bnnn)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op1nnn_JumpsToAddress) {
    emu::instruction_set::op1nnn(*state_, 0x1234);
    EXPECT_EQ(state_->program_counter, 0x234);
}

TEST_F(Chip8OpcodeTest, OpBnnn_JumpsToAddressPlusV0) {
    state_->V[0] = 0x10;
    emu::instruction_set::opBnnn(*state_, 0xB300);
    EXPECT_EQ(state_->program_counter, 0x310);
}

// ============================================================================
// Call Instruction (2nnn)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op2nnn_CallsSubroutine) {
    state_->program_counter = 0x200;

    emu::instruction_set::op2nnn(*state_, 0x2400);

    EXPECT_EQ(state_->stack.top(), 0x200);
    EXPECT_EQ(state_->program_counter, 0x400);
}

// ============================================================================
// Skip Instructions (3xkk, 4xkk, 5xy0, 9xy0)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op3xkk_SkipsIfVxEqualsKK) {
    state_->V[5] = 0x42;
    state_->program_counter = 0x200;

    emu::instruction_set::op3xkk(*state_, 0x3542);
    EXPECT_EQ(state_->program_counter, 0x202);
}

TEST_F(Chip8OpcodeTest, Op3xkk_DoesNotSkipIfVxNotEqualsKK) {
    state_->V[5] = 0x42;
    state_->program_counter = 0x200;

    emu::instruction_set::op3xkk(*state_, 0x3543);
    EXPECT_EQ(state_->program_counter, 0x200);
}

TEST_F(Chip8OpcodeTest, Op4xkk_SkipsIfVxNotEqualsKK) {
    state_->V[5] = 0x42;
    state_->program_counter = 0x200;

    emu::instruction_set::op4xkk(*state_, 0x4543);
    EXPECT_EQ(state_->program_counter, 0x202);
}

TEST_F(Chip8OpcodeTest, Op4xkk_DoesNotSkipIfVxEqualsKK) {
    state_->V[5] = 0x42;
    state_->program_counter = 0x200;

    emu::instruction_set::op4xkk(*state_, 0x4542);
    EXPECT_EQ(state_->program_counter, 0x200);
}

TEST_F(Chip8OpcodeTest, Op5xy0_SkipsIfVxEqualsVy) {
    state_->V[3] = 0x42;
    state_->V[7] = 0x42;
    state_->program_counter = 0x200;

    emu::instruction_set::op5xy0(*state_, 0x5370);
    EXPECT_EQ(state_->program_counter, 0x202);
}

TEST_F(Chip8OpcodeTest, Op5xy0_DoesNotSkipIfVxNotEqualsVy) {
    state_->V[3] = 0x42;
    state_->V[7] = 0x43;
    state_->program_counter = 0x200;

    emu::instruction_set::op5xy0(*state_, 0x5370);
    EXPECT_EQ(state_->program_counter, 0x200);
}

TEST_F(Chip8OpcodeTest, Op9xy0_SkipsIfVxNotEqualsVy) {
    state_->V[3] = 0x42;
    state_->V[7] = 0x43;
    state_->program_counter = 0x200;

    emu::instruction_set::op9xy0(*state_, 0x9370);
    EXPECT_EQ(state_->program_counter, 0x202);
}

TEST_F(Chip8OpcodeTest, Op9xy0_DoesNotSkipIfVxEqualsVy) {
    state_->V[3] = 0x42;
    state_->V[7] = 0x42;
    state_->program_counter = 0x200;

    emu::instruction_set::op9xy0(*state_, 0x9370);
    EXPECT_EQ(state_->program_counter, 0x200);
}

// ============================================================================
// Load/Set Instructions (6xkk, 8xy0, Annn)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op6xkk_SetsVxToKK) {
    emu::instruction_set::op6xkk(*state_, 0x6542);
    EXPECT_EQ(state_->V[5], 0x42);
}

TEST_F(Chip8OpcodeTest, Op8xy0_SetsVxToVy) {
    state_->V[7] = 0x99;
    emu::instruction_set::op8xy0(*state_, 0x8370);
    EXPECT_EQ(state_->V[3], 0x99);
}

TEST_F(Chip8OpcodeTest, OpAnnn_SetsIndexRegister) {
    emu::instruction_set::opAnnn(*state_, 0xA234);
    EXPECT_EQ(state_->index_register, 0x234);
}

// ============================================================================
// Arithmetic Instructions (7xkk, 8xy4, 8xy5, 8xy7)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op7xkk_AddsKKToVx) {
    state_->V[5] = 0x10;
    emu::instruction_set::op7xkk(*state_, 0x7520);
    EXPECT_EQ(state_->V[5], 0x30);
}

TEST_F(Chip8OpcodeTest, Op7xkk_WrapsOnOverflow) {
    state_->V[5] = 0xFF;
    emu::instruction_set::op7xkk(*state_, 0x7502);
    EXPECT_EQ(state_->V[5], 0x01);
}

TEST_F(Chip8OpcodeTest, Op8xy4_AddsVyToVxWithoutCarry) {
    state_->V[3] = 0x10;
    state_->V[7] = 0x20;

    emu::instruction_set::op8xy4(*state_, 0x8374);

    EXPECT_EQ(state_->V[3], 0x30);
    EXPECT_EQ(state_->V[0xF], 0x00);
}

TEST_F(Chip8OpcodeTest, Op8xy4_AddsVyToVxWithCarry) {
    state_->V[3] = 0xFF;
    state_->V[7] = 0xFF;

    emu::instruction_set::op8xy4(*state_, 0x8374);

    EXPECT_EQ(state_->V[3], 0xFE);
    EXPECT_EQ(state_->V[0xF], 0x01);
}

TEST_F(Chip8OpcodeTest, Op8xy5_SubtractsVyFromVxWithBorrow) {
    state_->V[3] = 0x30;
    state_->V[7] = 0x20;

    emu::instruction_set::op8xy5(*state_, 0x8375);

    EXPECT_EQ(state_->V[3], 0x10);
    EXPECT_EQ(state_->V[0xF], 0x01);  // No borrow
}

TEST_F(Chip8OpcodeTest, Op8xy5_SubtractsVyFromVxWithoutBorrow) {
    state_->V[3] = 0x20;
    state_->V[7] = 0x30;

    emu::instruction_set::op8xy5(*state_, 0x8375);

    EXPECT_EQ(state_->V[3], 0xF0);
    EXPECT_EQ(state_->V[0xF], 0x00);  // Borrow occurred
}

TEST_F(Chip8OpcodeTest, Op8xy7_SubtractsVxFromVyWithBorrow) {
    state_->V[3] = 0x20;
    state_->V[7] = 0x30;

    emu::instruction_set::op8xy7(*state_, 0x8377);

    EXPECT_EQ(state_->V[3], 0x10);
    EXPECT_EQ(state_->V[0xF], 0x01);  // No borrow
}

TEST_F(Chip8OpcodeTest, Op8xy7_SubtractsVxFromVyWithoutBorrow) {
    state_->V[3] = 0x30;
    state_->V[7] = 0x20;

    emu::instruction_set::op8xy7(*state_, 0x8377);

    EXPECT_EQ(state_->V[3], 0xF0);
    EXPECT_EQ(state_->V[0xF], 0x00);  // Borrow occurred
}

// ============================================================================
// Logical Instructions (8xy1, 8xy2, 8xy3)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op8xy1_ORsVxWithVy) {
    state_->V[3] = 0b10101010;
    state_->V[7] = 0b01010101;

    emu::instruction_set::op8xy1(*state_, 0x8371);

    EXPECT_EQ(state_->V[3], 0b11111111);
}

TEST_F(Chip8OpcodeTest, Op8xy2_ANDsVxWithVy) {
    state_->V[3] = 0b11110000;
    state_->V[7] = 0b10101010;

    emu::instruction_set::op8xy2(*state_, 0x8372);

    EXPECT_EQ(state_->V[3], 0b10100000);
}

TEST_F(Chip8OpcodeTest, Op8xy3_XORsVxWithVy) {
    state_->V[3] = 0b11110000;
    state_->V[7] = 0b10101010;

    emu::instruction_set::op8xy3(*state_, 0x8373);

    EXPECT_EQ(state_->V[3], 0b01011010);
}

// ============================================================================
// Shift Instructions (8xy6, 8xyE)
// ============================================================================

TEST_F(Chip8OpcodeTest, Op8xy6_ShiftsVxRight) {
    state_->V[3] = 0b10101010;

    emu::instruction_set::op8xy6(*state_, 0x8376);

    EXPECT_EQ(state_->V[3], 0b01010101);
    EXPECT_EQ(state_->V[0xF], 0x00);
}

TEST_F(Chip8OpcodeTest, Op8xy6_StoresLSBInVF) {
    state_->V[3] = 0b10101011;

    emu::instruction_set::op8xy6(*state_, 0x8376);

    EXPECT_EQ(state_->V[3], 0b01010101);
    EXPECT_EQ(state_->V[0xF], 0x01);
}

TEST_F(Chip8OpcodeTest, Op8xyE_ShiftsVxLeft) {
    state_->V[3] = 0b01010101;

    emu::instruction_set::op8xyE(*state_, 0x837E);

    EXPECT_EQ(state_->V[3], 0b10101010);
    EXPECT_EQ(state_->V[0xF], 0x00);
}

TEST_F(Chip8OpcodeTest, Op8xyE_StoresMSBInVF) {
    state_->V[3] = 0b10101010;

    emu::instruction_set::op8xyE(*state_, 0x837E);

    EXPECT_EQ(state_->V[3], 0b01010100);
    EXPECT_EQ(state_->V[0xF], 0x01);
}

// ============================================================================
// Random Instruction (Cxkk)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpCxkk_GeneratesRandomNumber) {
    emu::instruction_set::opCxkk(*state_, 0xC50F);
    // Result should be masked with 0x0F
    EXPECT_LE(state_->V[5], 0x0F);
}

// ============================================================================
// Display Instruction (Dxyn)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpDxyn_DrawsSprite) {
    state_->V[1] = 10;  // X coordinate
    state_->V[2] = 20;  // Y coordinate
    state_->index_register = 0x300;
    state_->memory[0x300] = 0b11110000;

    emu::instruction_set::opDxyn(*state_, 0xD123);  // Draw 3-byte sprite

    EXPECT_TRUE(state_->display.draw);
}

TEST_F(Chip8OpcodeTest, OpDxyn_DetectsCollision) {
    state_->V[1] = 0;
    state_->V[2] = 0;
    state_->index_register = 0x300;
    state_->memory[0x300] = 0xFF;

    // Set a pixel that will collide
    state_->display.buffer[0] = 1;

    emu::instruction_set::opDxyn(*state_, 0xD121);

    EXPECT_EQ(state_->V[0xF], 0x01);
}

// ============================================================================
// Keyboard Instructions (Ex9E, ExA1)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpEx9E_SkipsIfKeyPressed) {
    std::array<bool, SDL_SCANCODE_COUNT> dummy_keyboard{};
    dummy_keyboard[emu::keyboard::mapping(0x3)] = true;

    state_->V[5] = 0x3;
    state_->keyboard = dummy_keyboard.data();
    state_->program_counter = 0x200;

    emu::instruction_set::opEx9E(*state_, 0xE59E);

    EXPECT_EQ(state_->program_counter, 0x202);
}

TEST_F(Chip8OpcodeTest, OpEx9E_DoesNotSkipIfKeyNotPressed) {
    std::array<bool, SDL_SCANCODE_COUNT> dummy_keyboard{};

    state_->V[5] = 0x3;
    state_->keyboard = dummy_keyboard.data();
    state_->program_counter = 0x200;

    emu::instruction_set::opEx9E(*state_, 0xE59E);

    EXPECT_EQ(state_->program_counter, 0x200);
}

TEST_F(Chip8OpcodeTest, OpExA1_SkipsIfKeyNotPressed) {
    std::array<bool, SDL_SCANCODE_COUNT> dummy_keyboard{};

    state_->V[5] = 0x3;
    state_->keyboard = dummy_keyboard.data();
    state_->program_counter = 0x200;

    emu::instruction_set::opExA1(*state_, 0xE5A1);

    EXPECT_EQ(state_->program_counter, 0x202);
}

TEST_F(Chip8OpcodeTest, OpExA1_DoesNotSkipIfKeyPressed) {
    std::array<bool, SDL_SCANCODE_COUNT> dummy_keyboard{};
    dummy_keyboard[emu::keyboard::mapping(0x3)] = true;

    state_->V[5] = 0x3;
    state_->keyboard = dummy_keyboard.data();
    state_->program_counter = 0x200;

    emu::instruction_set::opExA1(*state_, 0xE5A1);

    EXPECT_EQ(state_->program_counter, 0x200);
}

// ============================================================================
// Timer Instructions (Fx07, Fx15, Fx18)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpFx07_ReadsDelayTimer) {
    state_->delay_timer = 0x42;

    emu::instruction_set::opFx07(*state_, 0xF507);

    EXPECT_EQ(state_->V[5], 0x42);
}

TEST_F(Chip8OpcodeTest, OpFx15_SetsDelayTimer) {
    state_->V[5] = 0x42;

    emu::instruction_set::opFx15(*state_, 0xF515);

    EXPECT_EQ(state_->delay_timer, 0x42);
}

TEST_F(Chip8OpcodeTest, OpFx18_SetsSoundTimer) {
    state_->V[5] = 0x42;

    emu::instruction_set::opFx18(*state_, 0xF518);

    EXPECT_EQ(state_->sound_timer, 0x42);
}

// ============================================================================
// Key Wait Instruction (Fx0A)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpFx0A_WaitsForKeyPress) {
    std::array<bool, SDL_SCANCODE_COUNT> dummy_keyboard{};

    state_->keyboard = dummy_keyboard.data();
    state_->program_counter = 0x202;

    emu::instruction_set::opFx0A(*state_, 0xF50A);

    EXPECT_EQ(state_->program_counter, 0x200);  // PC decremented
}

TEST_F(Chip8OpcodeTest, OpFx0A_StoresKeyWhenPressed) {
    std::array<bool, SDL_SCANCODE_COUNT> dummy_keyboard{};
    dummy_keyboard[emu::keyboard::mapping(0x5)] = true;

    state_->keyboard = dummy_keyboard.data();
    state_->program_counter = 0x200;

    emu::instruction_set::opFx0A(*state_, 0xF30A);

    EXPECT_EQ(state_->V[3], 0x5);
    EXPECT_EQ(state_->program_counter, 0x200);  // PC not decremented
}

// ============================================================================
// Index Register Instructions (Fx1E, Fx29)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpFx1E_AddsVxToIndexRegister) {
    state_->index_register = 0x200;
    state_->V[5] = 0x10;

    emu::instruction_set::opFx1E(*state_, 0xF51E);

    EXPECT_EQ(state_->index_register, 0x210);
}

TEST_F(Chip8OpcodeTest, OpFx29_LoadsFontLocation) {
    state_->V[5] = 0xA;

    emu::instruction_set::opFx29(*state_, 0xF529);

    EXPECT_EQ(state_->index_register,
              emu::font::kMemoryOffset + (0xA * emu::font::kSpriteSize));
}

// ============================================================================
// BCD Instruction (Fx33)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpFx33_StoresBCD) {
    state_->V[5] = 123;
    state_->index_register = 0x300;

    emu::instruction_set::opFx33(*state_, 0xF533);

    EXPECT_EQ(state_->memory[0x300], 1);
    EXPECT_EQ(state_->memory[0x301], 2);
    EXPECT_EQ(state_->memory[0x302], 3);
}

TEST_F(Chip8OpcodeTest, OpFx33_HandlesSingleDigit) {
    state_->V[5] = 7;
    state_->index_register = 0x300;

    emu::instruction_set::opFx33(*state_, 0xF533);

    EXPECT_EQ(state_->memory[0x300], 0);
    EXPECT_EQ(state_->memory[0x301], 0);
    EXPECT_EQ(state_->memory[0x302], 7);
}

// ============================================================================
// Memory Instructions (Fx55, Fx65)
// ============================================================================

TEST_F(Chip8OpcodeTest, OpFx55_StoresRegistersInMemory) {
    state_->index_register = 0x300;
    for (int i = 0; i <= 5; i++) {
        state_->V[i] = static_cast<std::uint8_t>(i * 10);
    }

    emu::instruction_set::opFx55(*state_, 0xF555);

    for (int i = 0; i <= 5; i++) {
        EXPECT_EQ(state_->memory[state_->index_register + i], i * 10);
    }
}

TEST_F(Chip8OpcodeTest, OpFx65_LoadsRegistersFromMemory) {
    state_->index_register = 0x300;
    for (int i = 0; i <= 5; i++) {
        state_->memory[state_->index_register + i] =
            static_cast<std::uint8_t>(i * 10);
    }

    emu::instruction_set::opFx65(*state_, 0xF565);

    for (int i = 0; i <= 5; i++) {
        EXPECT_EQ(state_->V[i], i * 10);
    }
}

}  // namespace emu::instruction_set::test

#endif /* TEST_INTRUCTION_SET_HPP */
