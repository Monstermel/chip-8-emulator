#ifndef CHIP_8_CHIP_8_HPP
#define CHIP_8_CHIP_8_HPP

#include <chrono>
#include <filesystem>

#include "chip_8/backend.hpp"
#include "chip_8/env.hpp"
#include "chip_8/frontend.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keyboard.h"

namespace emu {

class Chip8 {
    Env env_;
    Backend backend_;
    Frontend frontend_;
    std::chrono::nanoseconds step_accumulator_{0};
    std::chrono::nanoseconds timer_accumulator_{0};

    /**
     * @brief Coordinate a single interpreter cycle
     *
     */
    void cycle(const std::chrono::nanoseconds& delta) {
        // 1. CPU: ~700 Hz (1,428,571 ns per instruction)
        step_accumulator_ += delta;
        while (step_accumulator_ >= std::chrono::nanoseconds(1'428'571)) {
            step_accumulator_ -= std::chrono::nanoseconds(1'428'571);

            backend_.setKeyboard(SDL_GetKeyboardState(NULL));

            backend_.step();
        }

        // 2. Timers & Graphics: 60 Hz (16,666,666 ns per tick)
        timer_accumulator_ += delta;
        while (timer_accumulator_ >= std::chrono::nanoseconds(16'666'666)) {
            timer_accumulator_ -= std::chrono::nanoseconds(16'666'666);

            backend_.updateDelayTimer();

            frontend_.handleSound(backend_.getSoundTimer());

            auto& display = backend_.getDisplay();
            if (display.draw) {
                frontend_.renderDisplay(display);
                display.draw = false;
            }
        }
    }

   public:
    /**
     * @brief Load ROM into the backend
     */
    void load(const std::filesystem::path& path) { backend_.load(path); }

    /**
     * @brief Start the interpreter loop
     */
    void start() {
        bool running = true;
        auto last_time = std::chrono::steady_clock::now();

        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
            }

            auto now = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now - last_time);
            last_time = now;

            cycle(delta);
        }
    }
};

}  // namespace emu

#endif /* CHIP_8_CHIP_8_HPP */
