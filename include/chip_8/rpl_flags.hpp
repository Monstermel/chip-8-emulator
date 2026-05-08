#ifndef CHIP_8_RPL_FLAGS_HPP
#define CHIP_8_RPL_FLAGS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace emu::rpl_flags {

constexpr std::size_t kNum = 16;

using Type = std::array<std::uint8_t, kNum>;

}  // namespace emu::rpl_flags

#endif /* CHIP_8_RPL_FLAGS_HPP */
