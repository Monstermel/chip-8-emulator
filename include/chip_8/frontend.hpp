#ifndef CHIP_8_FRONTEND_HPP
#define CHIP_8_FRONTEND_HPP

#include <memory>
#include <stdexcept>

#include "chip_8/display.hpp"

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace emu {

// Bridge between emulator backend and emulator frotend (aka SDL)
class Frontend {
    using WindowDeleter =
        decltype([](SDL_Window* window) { SDL_DestroyWindow(window); });
    std::unique_ptr<SDL_Window, WindowDeleter> window_;

    using RendererDeleter =
        decltype([](SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); });
    std::unique_ptr<SDL_Renderer, RendererDeleter> renderer_;

   public:
    Frontend() {
        SDL_Window* raw_window{};
        SDL_Renderer* raw_renderer{};
        if (!SDL_CreateWindowAndRenderer(
                "Chip-8", display::kWidth * 10, display::kHeight * 10,
                SDL_WINDOW_RESIZABLE, &raw_window, &raw_renderer)) {
            throw std::runtime_error(SDL_GetError());
        }

        window_ = decltype(window_)(raw_window);
        renderer_ = decltype(renderer_)(raw_renderer);

        if (!SDL_SetRenderScale(renderer_.get(), 10.0F, 10.0F)) {
            throw std::runtime_error(SDL_GetError());
        }
    };

    // TODO: Add batch drawing
    void renderDisplay(const display::Type& display) {
        // Clear screen to black
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

        // Set drawing color to white (CHIP-8 foreground)
        SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);

        // Draw pixels
        for (std::size_t y = 0; y < display::kHeight; y++) {
            for (std::size_t x = 0; x < display::kWidth; x++) {
                if (display.buffer[x + (y * display::kWidth)] != 0U) {
                    // REVIEW: How expensive are these casts?
                    SDL_RenderPoint(renderer_.get(), static_cast<float>(x),
                                    static_cast<float>(y));
                }
            }
        }

        // Update screen
        SDL_RenderPresent(renderer_.get());
    }
};

};  // namespace emu

#endif /* CHIP_8_FRONTEND_HPP */
