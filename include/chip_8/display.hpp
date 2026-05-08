#ifndef CHIP_8_DISPLAY_HPP
#define CHIP_8_DISPLAY_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace emu::display {  // Registers metadata

constexpr std::size_t kLowWidth = 64;
constexpr std::size_t kLowHeight = 32;

constexpr std::size_t kHighWidth = 128;
constexpr std::size_t kHighHeight = 64;

enum class Mode : std::uint8_t { kLow = 0, kHigh = 1 };

struct Display {
    std::array<std::uint8_t, kHighWidth * kHighHeight> buffer{};
    bool draw{false};
    Mode mode{Mode::kLow};
};

using Type = Display;

}  // namespace emu::display

#endif /* CHIP_8_DISPLAY_HPP */
