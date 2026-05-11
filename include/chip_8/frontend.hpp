#ifndef CHIP_8_FRONTEND_HPP
#define CHIP_8_FRONTEND_HPP

#include <array>
#include <memory>

#include "chip_8/display.hpp"
#include "chip_8/error.hpp"

#include "SDL3/SDL_audio.h"
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

    using PaletteHandler =
        std::unique_ptr<SDL_Palette, decltype([](SDL_Palette* palette) {
                            SDL_DestroyPalette(palette);
                        })>;
    PaletteHandler palette_;

    using TextureHandler =
        std::unique_ptr<SDL_Texture, decltype([](SDL_Texture* texture) {
                            SDL_DestroyTexture(texture);
                        })>;
    TextureHandler texture_;

    bool audio_running_{false};

    static auto makeWindow(float scale, bool fullscreen) {
        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
        if (fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        SDL_Window* raw_window = SDL_CreateWindow(
            "Chip-8", static_cast<int>(display::kHighWidth * scale),
            static_cast<int>(display::kHighHeight * scale), flags);
        if (raw_window == nullptr) {
            throw FailedToSetupSDLError(SDL_GetError());
        }

        return WindowHandler{raw_window};
    }

    static auto makeRenderer(SDL_Window* window) {
        SDL_Renderer* raw_renderer = SDL_CreateRenderer(window, nullptr);
        if (raw_renderer == nullptr) {
            throw FailedToSetupSDLError(SDL_GetError());
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
            throw FailedToSetupSDLError(SDL_GetError());
        }

        return AudioStreamHandler{raw_audio_stream};
    }

   public:
    explicit Frontend(float scale = 10.0F, bool fullscreen = false)
        : window_(makeWindow(scale, fullscreen)),
          renderer_(makeRenderer(window_.get())),
          audio_stream_(makeAudioStream()),
          palette_(SDL_CreatePalette(2)),
          texture_(SDL_CreateTexture(renderer_.get(),
                                     SDL_PIXELFORMAT_INDEX8,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     static_cast<int>(display::kHighWidth),
                                     static_cast<int>(display::kHighHeight))) {
        std::array<SDL_Color, 2> colors = {
            SDL_Color{.r = 0, .g = 0, .b = 0, .a = 255},
            SDL_Color{.r = 255, .g = 255, .b = 255, .a = 255}};
        if (!SDL_SetPaletteColors(palette_.get(), colors.data(), 0, 2)) {
            throw FailedToSetupSDLError(SDL_GetError());
        }

        if (!SDL_SetTextureScaleMode(texture_.get(), SDL_SCALEMODE_NEAREST)) {
            throw FailedToSetupSDLError(SDL_GetError());
        }

        if (!SDL_SetTexturePalette(texture_.get(), palette_.get())) {
            throw FailedToSetupSDLError(SDL_GetError());
        }

        // Use logical presentation to handle scaling automatically
        if (!SDL_SetRenderLogicalPresentation(
                renderer_.get(), static_cast<int>(display::kHighWidth),
                static_cast<int>(display::kHighHeight),
                SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
            throw FailedToSetupSDLError(SDL_GetError());
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
            0, 0, static_cast<float>(display::kHighWidth),
            static_cast<float>(display::kHighHeight)};
        SDL_RenderFillRect(renderer_.get(), &kFullOverlay);

        // 2. Compact Box in the center
        constexpr float kBoxW = 48.0F;
        constexpr float kBoxH = 26.0F;
        constexpr float kBoxX =
            (static_cast<float>(display::kHighWidth) - kBoxW) / 2.0F;
        constexpr float kBoxY =
            (static_cast<float>(display::kHighHeight) - kBoxH) / 2.0F;

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
            float item_y =
                (kBoxY + 11.0F + static_cast<float>(i) * 5.0F) / 0.5F;

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

        SDL_RenderPresent(renderer_.get());
    }

    void renderDisplay(const display::Type& display, bool dimmed = false) {
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

        if (dimmed) {
            SDL_SetTextureColorMod(texture_.get(), 80, 80, 80);
        } else {
            SDL_SetTextureColorMod(texture_.get(), 255, 255, 255);
        }

        if (display.mode == display::Mode::kLow) {
            const SDL_Rect kSrcRect = {0, 0,
                                       static_cast<int>(display::kLowWidth),
                                       static_cast<int>(display::kLowHeight)};
            SDL_UpdateTexture(texture_.get(), &kSrcRect, display.buffer.data(),
                              static_cast<int>(display::kLowWidth));

            const SDL_FRect kSrcFRect = {
                0.0F, 0.0F, static_cast<float>(display::kLowWidth),
                static_cast<float>(display::kLowHeight)};
            SDL_RenderTexture(renderer_.get(), texture_.get(), &kSrcFRect,
                              nullptr);
        } else {
            SDL_UpdateTexture(texture_.get(), nullptr, display.buffer.data(),
                              static_cast<int>(display::kHighWidth));

            SDL_RenderTexture(renderer_.get(), texture_.get(), nullptr,
                              nullptr);
        }

        if (!dimmed) {
            SDL_RenderPresent(renderer_.get());
        }
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
