#ifndef CHIP_8_RPL_FLAGS_HPP
#define CHIP_8_RPL_FLAGS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace emu::rpl_flags {

constexpr std::size_t kNum = 16;

struct RPLFlags {
    std::array<std::uint8_t, kNum> flags{};
    bool dirty{false};
};

using Type = RPLFlags;

}  // namespace emu::rpl_flags

#endif /* CHIP_8_RPL_FLAGS_HPP */
