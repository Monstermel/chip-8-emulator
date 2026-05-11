#ifndef CHIP_8_INSTRUCTION_SET_HPP
#define CHIP_8_INSTRUCTION_SET_HPP

#include <cstdint>

#include "chip_8/chip_state.hpp"

namespace emu::instruction_set {

/**
 * @brief JMP to a host machine code - Treated as NOP
 *
 * @param bytecode
 */
void op0nnn(ChipState& /* not used */, const std::uint16_t /* not used */);

/**
 * @brief SCD nibble - Scroll display down n rows.
 *
 * @param bytecode
 */
void op00Cn(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief CLS - Clear the display.
 *
 * @param bytecode
 */
void op00E0(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief Return Call - The interpreter sets the program counter to the
 * address at the top of the stack, then subtracts 1 from the stack pointer.
 *
 * @param bytecode
 */
void op00EE(ChipState& state, const std::uint16_t /* not used */);

/**
 * @brief SCR - Scroll display right.
 *
 * @param bytecode
 */
void op00FB(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SCL - Scroll display left.
 *
 * @param bytecode
 */
void op00FC(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief Exit - Exit interpreter.
 *
 * @param bytecode
 */
void op00FD(ChipState& state, const std::uint16_t /* not used */);

/**
 * @brief Low - Set low resolution.
 *
 * @param bytecode
 */
void op00FE(ChipState& state, const std::uint16_t /* not used */);

/**
 * @brief High - Set high resolution.
 *
 * @param bytecode
 */
void op00FF(ChipState& state, const std::uint16_t /* not used */);

/**
 * @brief JMP to address - The interpreter sets the program counter to nnn.
 *
 * @param bytecode
 */
void op1nnn(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief Call address - The interpreter increments the stack pointer, then
 * puts the current PC on the top of the stack. The PC is then set to nnn.
 *
 * @param bytecode
 */
void op2nnn(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SE Vx, byte - Skip next bytecode if Vx = kk.
 *
 * @param bytecode
 */
void op3xkk(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SNE Vx, byte - Skip next bytecode if Vx != kk.
 *
 * @param bytecode
 */
void op4xkk(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SE Vx, Vy - Skip next bytecode if Vx = Vy.
 *
 * @param bytecode
 */
void op5xy0(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD Vx, byte - Set Vx = kk.
 *
 * @param bytecode
 */
void op6xkk(ChipState& state, const std::uint16_t bytecode);
/**
 * @brief ADD Vx, byte - Set Vx = Vx + kk.
 *
 * @param bytecode
 */
void op7xkk(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD Vx, Vy - Set Vx = Vy.
 *
 * @param bytecode
 */
void op8xy0(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief OR Vx, Vy - Set Vx = Vx OR Vy.
 *
 * @param bytecode
 */
void op8xy1(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief AND Vx, Vy - Set Vx = Vx AND Vy.
 *
 * @param bytecode
 */
void op8xy2(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief XOR Vx, Vy - Set Vx = Vx XOR Vy.
 *
 * @param bytecode
 */
void op8xy3(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry.
 *
 * @param bytecode
 */
void op8xy4(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow.
 *
 * @param bytecode
 */
void op8xy5(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SHR Vx {, Vy} - Set Vx = Vx SHR 1.
 *
 * @param bytecode
 */
void op8xy6(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow.
 *
 * @param bytecode
 */
void op8xy7(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SHL Vx {, Vy} - Set Vx = Vx SHL 1.
 *
 * @param bytecode
 */
void op8xyE(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SNE Vx, Vy - Skip next bytecode if Vx != Vy.
 *
 * @param bytecode
 */
void op9xy0(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD I, addr - Set I = nnn.
 *
 * @param bytecode
 */
void opAnnn(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief JP V0, addr - Jump to location nnn + V0.
 * @note COSMAC VIP behavior. Use V0 instead of Vx.
 * @todo Add option to recreate CHIP-48 and SUPER-CHIP. Use Vx instead of V0.
 * @param bytecode
 */
void opBnnn(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief RND Vx, byte - Set Vx = random byte AND kk.
 *
 * @param bytecode
 */
void opCxkk(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief DRW Vx, Vy, nibble - Display n-byte sprite starting at memory
 * location I at (Vx, Vy), set VF = collision.
 *
 * @param bytecode
 */
void opDxyn(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SKP Vx - Skip next bytecode if key with the value of Vx is
 * pressed.
 *
 * @param bytecode
 */
void opEx9E(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief SKNP Vx - Skip next bytecode if key with the value of Vx is not
 * pressed.
 *
 * @param bytecode
 */
void opExA1(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD Vx, DT - Set Vx = delay timer value.
 *
 * @param bytecode
 */
void opFx07(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD Vx, K - Wait for a key press, store the value of the key in Vx.
 *
 * @param bytecode
 */
void opFx0A(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD DT, Vx - Set delay timer = Vx.
 *
 * @param bytecode
 */
void opFx15(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD ST, Vx - Set sound timer = Vx.
 *
 * @param bytecode
 */
void opFx18(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief ADD I, Vx - Set I = I + Vx.
 *
 * @param bytecode
 */
void opFx1E(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD F, Vx - Set I = location of sprite for digit Vx.
 *
 * @param bytecode
 */
void opFx29(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD HF, Vx - Set I = location of 16x16 sprite for digit Vx.
 *
 * @param bytecode
 */
void opFx30(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD B, Vx - Store BCD representation of Vx in memory locations I,
 * I+1, and I+2.
 *
 * @param bytecode
 */
void opFx33(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD [I], Vx - Store registers V0 through Vx in memory starting at
 * location I.
 *
 * @param bytecode
 */
void opFx55(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief Fx65 - LD Vx, [I] - Load memory starting at location I into registers
 * V0 through Vx.
 *
 * @param bytecode
 */
void opFx65(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD R, Vx - Store registers V0 through Vx in RPL user flags
 * (X=0..7).
 *
 * @param bytecode
 */
void opFx75(ChipState& state, const std::uint16_t bytecode);

/**
 * @brief LD Vx, R - Load RPL user flags (X=0..7) into registers V0 through
 * Vx.
 *
 * @param bytecode
 */
void opFx85(ChipState& state, const std::uint16_t bytecode);

};  // namespace emu::instruction_set

#endif /* CHIP_8_INSTRUCTION_SET_HPP */
