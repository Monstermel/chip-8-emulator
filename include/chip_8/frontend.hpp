#ifndef CHIP_8_FRONTEND_HPP
#define CHIP_8_FRONTEND_HPP

#include <array>
#include <atomic>
#include <memory>
#include <stdexcept>

#include "SDL3/SDL_audio.h"
#include "chip_8/display.hpp"

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace emu {

// Bridge between emulator backend and emulator frotend (aka SDL)
class Frontend {
    using WindowHandler =
        std::unique_ptr<SDL_Window, decltype([](SDL_Window* window) {
                            SDL_DestroyWindow(window);
                        })>;
    WindowHandler window_;

    using RendererHandler =
        std::unique_ptr<SDL_Renderer, decltype([](SDL_Renderer* renderer) {
                            SDL_DestroyRenderer(renderer);
                        })>;
    RendererHandler renderer_;

    using AudioStreamHandler =
        std::unique_ptr<SDL_AudioStream,
                        decltype([](SDL_AudioStream* audio_stream) {
                            SDL_DestroyAudioStream(audio_stream);
                        })>;
    AudioStreamHandler audio_stream_;

    bool audio_running_{false};

    static auto makeWindow() {
        SDL_Window* raw_window =
            SDL_CreateWindow("Chip-8", display::kWidth * 10,
                             display::kHeight * 10, SDL_WINDOW_RESIZABLE);
        if (raw_window == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }

        return WindowHandler{raw_window};
    }

    static auto makeRenderer(SDL_Window* window) {
        SDL_Renderer* raw_renderer = SDL_CreateRenderer(window, nullptr);
        if (raw_renderer == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }

        return RendererHandler{raw_renderer};
    }

    static auto makeAudioStream() {
        SDL_AudioSpec spec{.format = SDL_AUDIO_U8, .channels = 1, .freq = 8000};

        // 8000Hz / 440Hz = 18.1818... samples per cycle.
        // 800 samples is exactly 44 cycles, which perfectly loops without
        // clicking.
        static constexpr auto kAudioBuffer = []() consteval {
            std::array<std::uint8_t, 800> arr{};
            float phase = 0.0F;
            for (int i = 0; i < 800; ++i) {
                phase += 440.0F / 8000.0F;
                if (phase > 1.0F) {
                    phase -= 1.0F;
                }
                // SDL_AUDIO_U8 silence is 128. Amplitude of 32 -> 160 and 96
                arr[i] = (phase < 0.5F) ? 160 : 96;
            }
            return arr;
        }();

        auto callback = [](void* /*userdata*/, SDL_AudioStream* stream,
                           int additional_amount, int /*total_amount*/) {
            // Push complete 800-byte chunks (exactly 44 cycles).
            // By always pushing complete cycles, we avoid needing any state!
            while (additional_amount > 0) {
                SDL_PutAudioStreamData(stream, kAudioBuffer.data(),
                                       kAudioBuffer.size());
                additional_amount -= kAudioBuffer.size();
            }
        };

        SDL_AudioStream* raw_audio_stream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, callback, nullptr);
        if (raw_audio_stream == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }

        return AudioStreamHandler{raw_audio_stream};
    }

   public:
    Frontend()
        : window_(makeWindow()),
          renderer_(makeRenderer(window_.get())),
          audio_stream_(makeAudioStream()) {
        if (!SDL_SetRenderScale(renderer_.get(), 10.0F, 10.0F)) {
            throw std::runtime_error(SDL_GetError());
        }
    }

    void renderDisplay(const display::Type& display) {
        // Clear screen to black
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

        // Set drawing color to white (CHIP-8 foreground)
        SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);

        // Batch pixels
        int count = 0;
        std::array<SDL_FPoint, display::kWidth * display::kHeight> points{};

        for (std::size_t y = 0; y < display::kHeight; y++) {
            for (std::size_t x = 0; x < display::kWidth; x++) {
                if (display.buffer[x + (y * display::kWidth)] != 0U) {
                    points[count++] = {.x = static_cast<float>(x),
                                       .y = static_cast<float>(y)};
                }
            }
        }

        if (count > 0) {
            SDL_RenderPoints(renderer_.get(), points.data(), count);
        }

        // Update screen
        SDL_RenderPresent(renderer_.get());
    }

    void handleSound(std::atomic<std::uint8_t>& sound_timer) {
        if (sound_timer > 0) {
            if (!audio_running_) {
                SDL_ResumeAudioStreamDevice(audio_stream_.get());
                audio_running_ = true;
            }

            sound_timer -= 1;
        } else {
            if (audio_running_) {
                SDL_PauseAudioStreamDevice(audio_stream_.get());
                audio_running_ = false;
            }
        }
    };

};  // namespace emu

}  // namespace emu

#endif /* CHIP_8_FRONTEND_HPP */
