#ifndef CHIP_8_STACK_HPP
#define CHIP_8_STACK_HPP

#include <array>
#include <cstdint>
#include "chip_8/error.hpp"

namespace emu::stack {

constexpr std::size_t kSize = 16;

class Stack {
    std::array<std::uint16_t, kSize> stack_{};
    std::uint8_t stack_pointer_{0};

   public:
    void push(const std::uint16_t value) {
        if (stack_pointer_ >= kSize) {
            throw emu::StackOverflowError("Stack overflow on push");
        }

        stack_[stack_pointer_++] = value;
    }

    std::uint16_t pop() {
        if (stack_pointer_ == 0) {
            throw emu::StackUnderflowError("Stack underflow on pop");
        }

        return stack_[--stack_pointer_];
    }
};

using Type = Stack;

}  // namespace emu::stack

#endif /* CHIP_8_STACK_HPP */
