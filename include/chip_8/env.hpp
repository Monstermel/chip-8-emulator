#ifndef CHIP_8_ENV_HPP
#define CHIP_8_ENV_HPP

#include "chip_8/error.hpp"

#include "SDL3/SDL_init.h"

namespace emu {

class Env {
   public:
    Env() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            throw FailedToSetupSDLError(SDL_GetError());
        }
    }

    ~Env() { SDL_Quit(); }

    Env(Env&&) = delete;
    Env(const Env&) = delete;
    Env& operator=(Env&&) = delete;
    Env& operator=(const Env&) = delete;
};

}  // namespace emu

#endif /* CHIP_8_ENV_HPP */
