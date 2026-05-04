#ifndef CHIP_8_ERROR_HANDLING_HPP
#define CHIP_8_ERROR_HANDLING_HPP

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

namespace emu {

class InvalidInstructionError : public std::runtime_error {
   public:
    explicit InvalidInstructionError()
        : std::runtime_error("Invalid instruction") {};
    explicit InvalidInstructionError(const std::uint16_t bytecode)
        : std::runtime_error(
              std::format("Invalid instruction: 0x{:04X}", bytecode)) {};
    explicit InvalidInstructionError(const std::string& message)
        : std::runtime_error(message) {}
};

class StackUnderflowError : public std::runtime_error {
   public:
    explicit StackUnderflowError() : std::runtime_error("Stack underflow") {};
    explicit StackUnderflowError(const std::string& message)
        : std::runtime_error(message) {}
};

class StackOverflowError : public std::runtime_error {
   public:
    explicit StackOverflowError() : std::runtime_error("Stack overflow") {};
    explicit StackOverflowError(const std::string& message)
        : std::runtime_error(message) {}
};

}  // namespace emu

#endif /* CHIP_8_ERROR_HANDLING_HPP */
