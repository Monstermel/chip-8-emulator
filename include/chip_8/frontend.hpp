#ifndef CHIP_8_FRONTEND_HPP
#define CHIP_8_FRONTEND_HPP

#include <array>
#include <atomic>
#include <memory>
#include <stdexcept>

#include "SDL3/SDL_audio.h"
#include "chip_8/display.hpp"

#include "SDL3/SDL_mouse.h"
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

    static auto makeWindow(float scale, bool fullscreen) {
        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
        if (fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        SDL_Window* raw_window = SDL_CreateWindow(
            "Chip-8", static_cast<int>(display::kWidth * scale),
            static_cast<int>(display::kHeight * scale), flags);
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

        static constexpr auto kAudioBuffer = []() consteval {
            std::array<std::uint8_t, 800> arr{};
            float phase = 0.0F;
            for (int i = 0; i < 800; ++i) {
                phase += 440.0F / 8000.0F;
                if (phase > 1.0F) {
                    phase -= 1.0F;
                }
                arr[i] = (phase < 0.5F) ? 160 : 96;
            }
            return arr;
        }();

        auto callback = [](void* /*userdata*/, SDL_AudioStream* stream,
                           int additional_amount, int /*total_amount*/) {
            while (additional_amount > 0) {
                SDL_PutAudioStreamData(stream, kAudioBuffer.data(),
                                       static_cast<int>(kAudioBuffer.size()));
                additional_amount -= static_cast<int>(kAudioBuffer.size());
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
    Frontend(float scale = 10.0F, bool fullscreen = false)
        : window_(makeWindow(scale, fullscreen)),
          renderer_(makeRenderer(window_.get())),
          audio_stream_(makeAudioStream()) {
        // Use logical presentation to handle scaling automatically
        if (!SDL_SetRenderLogicalPresentation(
                renderer_.get(), static_cast<int>(display::kWidth),
                static_cast<int>(display::kHeight),
                SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
            throw std::runtime_error(SDL_GetError());
        }

        // Set scaling quality to nearest pixel for sharp graphics
        if (!SDL_SetRenderScale(renderer_.get(), 1.0F, 1.0F)) {
            // Not strictly necessary if logical presentation is used,
            // but helps ensure 1:1 logical units
        }

        if (fullscreen) {
            SDL_HideCursor();
        }
    }

    void renderMenu(int selected_item) {
        // 1. Semi-transparent overlay (full screen)
        SDL_SetRenderDrawBlendMode(renderer_.get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 160);
        constexpr SDL_FRect kFullOverlay = {
            0, 0, static_cast<float>(display::kWidth),
            static_cast<float>(display::kHeight)};
        SDL_RenderFillRect(renderer_.get(), &kFullOverlay);

        // 2. Compact Box in the center
        constexpr float kBoxW = 48.0F;
        constexpr float kBoxH = 26.0F;
        constexpr float kBoxX =
            (static_cast<float>(display::kWidth) - kBoxW) / 2.0F;
        constexpr float kBoxY =
            (static_cast<float>(display::kHeight) - kBoxH) / 2.0F;

        // Shadow
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 100);
        constexpr SDL_FRect kShadow = {kBoxX + 1.0F, kBoxY + 1.0F, kBoxW,
                                       kBoxH};
        SDL_RenderFillRect(renderer_.get(), &kShadow);

        // Main Box
        SDL_SetRenderDrawColor(renderer_.get(), 25, 25, 35, 245);
        constexpr SDL_FRect kBox = {kBoxX, kBoxY, kBoxW, kBoxH};
        SDL_RenderFillRect(renderer_.get(), &kBox);

        // Border
        SDL_SetRenderDrawColor(renderer_.get(), 80, 80, 120, 255);
        SDL_RenderRect(renderer_.get(), &kBox);

        // 3. Draw Title (Bigger - Scale 0.75)
        SDL_SetRenderScale(renderer_.get(), 0.75F, 0.75F);
        SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);

        // "PAUSED" is 6 chars * 8 * 0.75 = 36 wide.
        // Centered in box: kBoxX + (48 - 36)/2 = kBoxX + 6.
        // Scaled coordinate: (kBoxX + 6) / 0.75
        SDL_RenderDebugText(renderer_.get(), (kBoxX + 6.0F) / 0.75F,
                            (kBoxY + 2.0F) / 0.75F, "PAUSED");

        // 4. Menu Items (Smaller - Scale 0.5)
        SDL_SetRenderScale(renderer_.get(), 0.5F, 0.5F);
        constexpr std::array<const char*, 3> kItems{"RESUME", "RESET", "QUIT"};
        constexpr std::array<float, 3> kXOffsets{12.0F, 14.0F,
                                                 16.0F};  // (48 - width)/2

        for (int i = 0; i < 3; ++i) {
            float item_x = (kBoxX + kXOffsets[i]) / 0.5F;
            float item_y = (kBoxY + 11.0F + i * 5.0F) / 0.5F;

            if (i == selected_item) {
                SDL_SetRenderDrawColor(renderer_.get(), 0, 255, 180, 255);
                SDL_RenderDebugText(renderer_.get(), item_x - 12.0F, item_y,
                                    ">");
                SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer_.get(), 170, 170, 170, 255);
            }
            SDL_RenderDebugText(renderer_.get(), item_x, item_y, kItems[i]);
        }

        // Reset scale for next frame
        SDL_SetRenderScale(renderer_.get(), 1.0F, 1.0F);
    }

    void renderDisplay(const display::Type& display,
                       bool paused = false,
                       int selected_item = 0) {
        int count = 0;
        std::array<SDL_FPoint, display::kWidth * display::kHeight> points{};

        // TODO: I am pretty sure that instructions can do the drawing directly
        // on points, so we get a zero-copy execution path
        for (std::size_t y = 0; y < display::kHeight; y++) {
            for (std::size_t x = 0; x < display::kWidth; x++) {
                if (display.buffer[x + (y * display::kWidth)] != 0U) {
                    points[count++] = {.x = static_cast<float>(x),
                                       .y = static_cast<float>(y)};
                }
            }
        }

        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

        if (paused) {
            SDL_SetRenderDrawColor(renderer_.get(), 80, 80, 80, 255);
        } else {
            SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);
        }

        if (count > 0) {
            SDL_RenderPoints(renderer_.get(), points.data(), count);
        }

        if (paused) {
            renderMenu(selected_item);
        }

        SDL_RenderPresent(renderer_.get());
    }

    void handleSound(std::uint8_t& sound_timer) {
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
};

}  // namespace emu

#endif /* CHIP_8_FRONTEND_HPP */
