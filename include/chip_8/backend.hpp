#ifndef CHIP_8_BACKEND_HPP
#define CHIP_8_BACKEND_HPP

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "chip_8/chip_state.hpp"
#include "chip_8/error.hpp"
#include "chip_8/instruction_set.hpp"
#include "chip_8/utility.hpp"

namespace emu {

class Backend {
    ChipState state_;
    // ROM buffer for reset operations
    std::array<std::uint8_t, memory::kSize - memory::kProgramSpaceOffset>
        rom_buffer_{};
    std::filesystem::path rom_path_;

    void saveRplFlags() {
        if (rom_path_.empty() || !state_.rpl.dirty) {
            return;
        }

        auto flags_path = rom_path_;
        flags_path.replace_extension(".flags");

        std::ofstream file(flags_path, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(state_.rpl.flags.data()),
                       state_.rpl.flags.size());
            state_.rpl.dirty = false;
        }
    }

    void loadRplFlags() {
        if (rom_path_.empty()) {
            return;
        }

        auto flags_path = rom_path_;
        flags_path.replace_extension(".flags");

        if (std::filesystem::exists(flags_path)) {
            std::ifstream file(flags_path, std::ios::binary);

            if (!file.is_open()) {
                throw FailedToLoadRPLFlagsError();
            }

            file.read(reinterpret_cast<char*>(state_.rpl.flags.data()),
                      state_.rpl.flags.size());
            state_.rpl.dirty = false;
        }
    }

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
    void handleGroup0(const std::uint16_t bytecode) {
        switch (bytecode & 0x0FFFU) {
            case 0x00E0:
                instruction_set::op00E0(state_, bytecode);
                return;
            case 0x00EE:
                instruction_set::op00EE(state_, bytecode);
                return;
            case 0x00FB:
                instruction_set::op00FB(state_, bytecode);
                return;
            case 0x00FC:
                instruction_set::op00FC(state_, bytecode);
                return;
            case 0x00FD:
                instruction_set::op00FD(state_, bytecode);
                return;
            case 0x00FE:
                instruction_set::op00FE(state_, bytecode);
                return;
            case 0x00FF:
                instruction_set::op00FF(state_, bytecode);
                return;
            default:
                switch (bytecode & 0x00F0U) {
                    case 0x00C0:
                        instruction_set::op00Cn(state_, bytecode);
                        return;
                    default:
                        instruction_set::op0nnn(state_, bytecode);
                        return;
                }
        }
    }

    /**
     * @brief Map bytecode to its instruction under 8xxx group
     *
     * @param bytecode
     * @return instruction_set::Instruction
     */
    void handleGroup8(const std::uint16_t bytecode) {
        switch (bytecode & 0x000FU) {
            case 0x0000:
                instruction_set::op8xy0(state_, bytecode);
                return;
            case 0x0001:
                instruction_set::op8xy1(state_, bytecode);
                return;
            case 0x0002:
                instruction_set::op8xy2(state_, bytecode);
                return;
            case 0x0003:
                instruction_set::op8xy3(state_, bytecode);
                return;
            case 0x0004:
                instruction_set::op8xy4(state_, bytecode);
                return;
            case 0x0005:
                instruction_set::op8xy5(state_, bytecode);
                return;
            case 0x0006:
                instruction_set::op8xy6(state_, bytecode);
                return;
            case 0x0007:
                instruction_set::op8xy7(state_, bytecode);
                return;
            case 0x000E:
                instruction_set::op8xyE(state_, bytecode);
                return;
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
    void handleGroupE(const std::uint16_t bytecode) {
        switch (bytecode & 0x00FFU) {
            case 0x009E:
                instruction_set::opEx9E(state_, bytecode);
                return;
            case 0x00A1:
                instruction_set::opExA1(state_, bytecode);
                return;
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
    void handleGroupF(const std::uint16_t bytecode) {
        switch (bytecode & 0x00FFU) {
            case 0x0007:
                instruction_set::opFx07(state_, bytecode);
                return;
            case 0x000A:
                instruction_set::opFx0A(state_, bytecode);
                return;
            case 0x0015:
                instruction_set::opFx15(state_, bytecode);
                return;
            case 0x0018:
                instruction_set::opFx18(state_, bytecode);
                return;
            case 0x001E:
                instruction_set::opFx1E(state_, bytecode);
                return;
            case 0x0029:
                instruction_set::opFx29(state_, bytecode);
                return;
            case 0x0030:
                instruction_set::opFx30(state_, bytecode);
                return;
            case 0x0033:
                instruction_set::opFx33(state_, bytecode);
                return;
            case 0x0055:
                instruction_set::opFx55(state_, bytecode);
                return;
            case 0x0065:
                instruction_set::opFx65(state_, bytecode);
                return;
            case 0x0075:
                instruction_set::opFx75(state_, bytecode);
                return;
            case 0x0085:
                instruction_set::opFx85(state_, bytecode);
                return;
            default:
                throw InvalidInstructionError(bytecode);
        }
    }

    /**
     * @brief Map a bytecode to its corresponding instruction and execute it
     * @param bytecode
     */
    void execute(const std::uint16_t bytecode) {
        switch (bytecode & 0xF000U) {
            case 0x0000:
                handleGroup0(bytecode);
                return;
            case 0x1000:
                instruction_set::op1nnn(state_, bytecode);
                return;
            case 0x2000:
                instruction_set::op2nnn(state_, bytecode);
                return;
            case 0x3000:
                instruction_set::op3xkk(state_, bytecode);
                return;
            case 0x4000:
                instruction_set::op4xkk(state_, bytecode);
                return;
            case 0x5000:
                instruction_set::op5xy0(state_, bytecode);
                return;
            case 0x6000:
                instruction_set::op6xkk(state_, bytecode);
                return;
            case 0x7000:
                instruction_set::op7xkk(state_, bytecode);
                return;
            case 0x8000:
                handleGroup8(bytecode);
                return;
            case 0x9000:
                instruction_set::op9xy0(state_, bytecode);
                return;
            case 0xA000:
                instruction_set::opAnnn(state_, bytecode);
                return;
            case 0xB000:
                instruction_set::opBnnn(state_, bytecode);
                return;
            case 0xC000:
                instruction_set::opCxkk(state_, bytecode);
                return;
            case 0xD000:
                instruction_set::opDxyn(state_, bytecode);
                return;
            case 0xE000:
                handleGroupE(bytecode);
                return;
            case 0xF000:
                handleGroupF(bytecode);
                return;
            default:
                throw InvalidInstructionError(bytecode);
        }
    }

   public:
    explicit Backend(const Mode mode = Mode::kSuperChip) { state_.mode = mode; }

    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend(const Backend&&) = delete;
    Backend&& operator=(const Backend&&) = delete;

    /**
     * @brief Load ROM into memory
     */
    void load(const std::filesystem::path& path) {
        rom_path_ = path;
        // Open ROM
        std::ifstream file(path, std::ifstream::ate | std::ifstream::binary);
        if (!file.is_open()) {
            throw FailedToLoadROMError();
        }

        // Get size of file
        std::streamsize size = file.tellg();
        file.seekg(std::ifstream::beg);

        // Store ROM data for resets
        if (!file.read(reinterpret_cast<char*>(rom_buffer_.data()), size)) {
            throw FailedToReadROMError();
        }

        // Apply ROM to memory
        std::ranges::copy(rom_buffer_,
                          state_.memory.begin() + memory::kProgramSpaceOffset);

        // Load persistent flags
        loadRplFlags();
    }

    void step() {
        const auto kBytecode = fetch();
        execute(kBytecode);
    }

    ~Backend() { saveRplFlags(); }

    void setKeyboard(const bool* key_states) noexcept {
        state_.keyboard = key_states;
    }

    void updateDelayTimer() {
        if (state_.delay_timer > 0) {
            state_.delay_timer -= 1;
        }
    }

    display::Type& getDisplay() noexcept { return state_.display; }

    std::uint8_t& getSoundTimer() noexcept { return state_.sound_timer; }

    [[nodiscard]] bool getExitFlag() const noexcept {
        return state_.should_exit;
    }

    /**
     * @brief Reset the emulator state
     */
    void reset() noexcept {
        // Preserve mode, exit flag, and RPL flags across resets
        const auto kCurrentMode = state_.mode;
        const auto kShouldExit = state_.should_exit;
        const auto kRplFlags = state_.rpl;

        state_ = ChipState();

        state_.mode = kCurrentMode;
        state_.should_exit = kShouldExit;
        state_.rpl = kRplFlags;
        state_.display.draw = true;

        // Re-apply ROM
        std::ranges::copy(rom_buffer_,
                          state_.memory.begin() + memory::kProgramSpaceOffset);
    }
};

}  // namespace emu

#endif /* CHIP_8_BACKEND_HPP */
