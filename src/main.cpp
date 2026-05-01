#include <cassert>
#include <exception>
#include <memory>

#include "chip_8/chip_8.hpp"

#define SDL_MAIN_USE_CALLBACKS 1
#include "SDL3/SDL.h"  // IWYU pragma: keep
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_main.h"

static std::unique_ptr<emu::Chip8> g_interpreter;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** /*appstate*/, int /*argc*/, char* /*argv*/[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to init sdl: %s",
                     SDL_GetError());
        return SDL_APP_FAILURE;
    }

    try {
        g_interpreter = std::make_unique<emu::Chip8>();
    } catch (const std::exception& error) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "Failed to init emulator: %s",
                     error.what());
        return SDL_APP_FAILURE;
    }

    if (g_interpreter->load() != 0) {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* /*appstate*/, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                                 */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* /*appstate*/) {
    try {
        g_interpreter->cycle();
    } catch (const std::exception& error) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Chip8::cycle failed: %s",
                     error.what());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
    SDL_Quit();
}
