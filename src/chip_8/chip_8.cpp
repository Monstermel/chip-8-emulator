#include "chip_8/chip_8.hpp"

#include <fstream>
#include <ios>
#include <iterator>

#include "chip_8/chip_state.hpp"

namespace emu {

// TODO: Add cxxopts
int Chip8::load() {
    // Open test ROM
    std::ifstream file("roms/snake.ch8",
                       // NOLINTNEXTLINE (hicpp-signed-bitwise)
                       std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open()) {
        return -1;
    }

    // Get size of file
    std::streamsize size = file.tellg();
    // Set file at beggining
    file.seekg(std::ifstream::beg);

    // Load test ROM into memory at kProgramSpaceOffset
    // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
    if (!file.read(std::next(reinterpret_cast<char*>(state_.memory.data()),
                             memory::kProgramSpaceOffset),
                   size)) {
        return -1;
    }

    return 0;
}

}  // namespace emu