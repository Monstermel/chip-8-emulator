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
    std::chrono::nanoseconds step_interval_;
    int menu_index_{0};
    bool running_{true};
    bool paused_{false};

    /**
     * @brief Coordinate a single interpreter cycle
     *
     */
    void cycle(const std::chrono::nanoseconds& delta) {
        // 1. CPU: Variable Speed
        step_accumulator_ += delta;
        while (step_accumulator_ >= step_interval_) {
            step_accumulator_ -= step_interval_;

            backend_.setKeyboard(SDL_GetKeyboardState(nullptr));

            backend_.step();
        }

        // 2. Timers & Graphics: Fixed 60 Hz (16,666,666 ns per tick)
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
     * @brief Construct a new Chip8 object with optional configuration
     *
     * @param speed_hz Instruction execution speed in Hz (default: 700)
     * @param scale Window scale factor (default: 10.0)
     * @param fullscreen Enable fullscreen mode (default: false)
     */
    explicit Chip8(int speed_hz = 700,
                   bool fullscreen = false,
                   float scale = 10.0F)
        : frontend_(scale, fullscreen),
          step_interval_(std::chrono::nanoseconds(1'000'000'000 / speed_hz)) {}

    /**
     * @brief Load ROM into the backend
     */
    void load(const std::filesystem::path& path) { backend_.load(path); }

    void eventHandler(const SDL_Event& event) {
        if (event.type == SDL_EVENT_QUIT) {
            running_ = false;
        } else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            paused_ = true;
            std::uint8_t zero = 0;
            frontend_.handleSound(zero);
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                paused_ = !paused_;
                if (paused_) {
                    std::uint8_t zero = 0;
                    frontend_.handleSound(zero);
                }
            }

            if (paused_) {
                if (event.key.key == SDLK_UP) {
                    menu_index_ = (menu_index_ + 2) % 3;
                } else if (event.key.key == SDLK_DOWN) {
                    menu_index_ = (menu_index_ + 1) % 3;
                } else if (event.key.key == SDLK_RETURN ||
                           event.key.key == SDLK_SPACE) {
                    switch (menu_index_) {
                        case 0:
                            paused_ = false;
                            break;
                        case 1:
                            backend_.reset();
                            paused_ = false;
                            break;
                        case 2:
                            running_ = false;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    /**
     * @brief Start the interpreter loop
     */
    void start() {
        auto last_time = std::chrono::steady_clock::now();

        while (running_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                eventHandler(event);
            }

            auto now = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now - last_time);
            last_time = now;

            if (paused_) {
                frontend_.renderDisplay(backend_.getDisplay(), true);
                frontend_.renderMenu(menu_index_);
            } else {
                cycle(delta);
            }

            if (backend_.getExitFlag()) {
                running_ = false;
            }
        }
    }
};

}  // namespace emu

#endif /* CHIP_8_CHIP_8_HPP */
