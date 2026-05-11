#ifndef CHIP_8_ERROR_HPP
#define CHIP_8_ERROR_HPP

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
        : std::runtime_error("Invalid instruction: " + message) {}
};

class StackUnderflowError : public std::runtime_error {
   public:
    explicit StackUnderflowError() : std::runtime_error("Stack underflow") {};
    explicit StackUnderflowError(const std::string& message)
        : std::runtime_error("Stack underflow: " + message) {}
};

class StackOverflowError : public std::runtime_error {
   public:
    explicit StackOverflowError() : std::runtime_error("Stack overflow") {};
    explicit StackOverflowError(const std::string& message)
        : std::runtime_error("Stack overflow: " + message) {}
};

class FailedToSetupSDLError : public std::runtime_error {
   public:
    explicit FailedToSetupSDLError()
        : std::runtime_error("Failed to setup SDL") {};
    explicit FailedToSetupSDLError(const std::string& message)
        : std::runtime_error("Failed to setup SDL: " + message) {}
};

class FailedToLoadRPLFlagsError : public std::runtime_error {
   public:
    explicit FailedToLoadRPLFlagsError()
        : std::runtime_error("Failed to load RPL flags") {};
    explicit FailedToLoadRPLFlagsError(const std::string& message)
        : std::runtime_error("Failed to load RPL flags: " + message) {}
};

class FailedToLoadROMError : public std::runtime_error {
   public:
    explicit FailedToLoadROMError()
        : std::runtime_error("Failed to load ROM") {};
    explicit FailedToLoadROMError(const std::string& message)
        : std::runtime_error("Failed to load ROM: " + message) {}
};

class FailedToReadROMError : public std::runtime_error {
   public:
    explicit FailedToReadROMError()
        : std::runtime_error("Failed to read ROM") {};
    explicit FailedToReadROMError(const std::string& message)
        : std::runtime_error("Failed to read ROM: " + message) {}
};

class InvalidModeError : public std::invalid_argument {
   public:
    explicit InvalidModeError() : std::invalid_argument("Invalid mode") {};
    explicit InvalidModeError(const std::string& message)
        : std::invalid_argument("Invalid mode: " + message) {}
};

class NullKeyboardError : public std::runtime_error {
   public:
    explicit NullKeyboardError()
        : std::runtime_error("Keyboard state is null") {};
    explicit NullKeyboardError(const std::string& message)
        : std::runtime_error("Keyboard state is null in " + message) {}
};

class InvalidSpriteLoadError : public std::runtime_error {
   public:
    explicit InvalidSpriteLoadError()
        : std::runtime_error("Invalid sprite load") {};
    explicit InvalidSpriteLoadError(const std::string& message)
        : std::runtime_error("Invalid sprite load in " + message) {}
};

}  // namespace emu

#endif /* CHIP_8_ERROR_HPP */
