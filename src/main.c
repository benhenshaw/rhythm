//
// main.c
//
// This file contains:
//     - Program entry point
//     - Initialisation for graphics and audio.
//     - Frame loop.
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>

#define POOL_STATIC_ALLOCATE
#define POOL_STATIC_PERSIST_BYTE_COUNT (64 * 1000 * 1000)

#include "common.c"
#include "memory.c"
#include "graphics.c"
#include "audio.c"
#include "assets.c"
#include "scene.c"

int main(int argument_count, char ** arguments) {
    // TODO: Error handling for init.

    setbuf(stdout, 0);

    init_memory_pools(megabytes(64), megabytes(32), megabytes(8));

    SDL_Init(SDL_INIT_EVERYTHING);

    //
    // Init graphics.
    //

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);

    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture * screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    pixels = pool_alloc(PERSIST_POOL, WIDTH * HEIGHT * sizeof(u32));

    SDL_ShowCursor(false);

    //
    // Init audio.
    //

    int audio_sample_count = (48000 / 60) * 2;
    // int audio_sample_count = 100;
    f32 * audio_samples = pool_alloc(PERSIST_POOL,
        audio_sample_count * sizeof(f32));

    SDL_AudioSpec audio_output_spec = {
        .freq = 48000,
        .format = AUDIO_F32,
        .channels = 2,
        .samples = 1,
    };

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, false,
        &audio_output_spec, NULL, 0);

    SDL_PauseAudioDevice(audio_device, false);

    //
    // Set up the timers.
    //

    u64 previous_counter_ticks = SDL_GetPerformanceCounter();
    f32 counter_ticks_per_second = SDL_GetPerformanceFrequency();

    //
    // Debug.
    //

    Image font_image = load_pam(PERSIST_POOL, "../assets/font.pam");
    Font debug_font = {
        .pixels = font_image.pixels,
        .char_width  = 6,
        .char_height = 12,
    };

    //
    // Start the game.
    //

    set_scene(heart_scene);

    while (true) {
        f32 delta_time = (SDL_GetPerformanceCounter() - previous_counter_ticks)
                            / counter_ticks_per_second;
        previous_counter_ticks = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                print_memory_stats();
                exit(0);
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                if (!event.key.repeat) {
                    SDL_Scancode sc = event.key.keysym.scancode;
                    if (sc == SDL_SCANCODE_LSHIFT) {
                        current_scene.input(current_scene.state, 0, event.key.state);
                    } else if (sc == SDL_SCANCODE_RSHIFT) {
                        current_scene.input(current_scene.state, 1, event.key.state);
                    }

                    // DEBUG scene switching.
                    else if (sc == SDL_SCANCODE_1) {
                        set_scene(menu_scene);
                    } else if (sc == SDL_SCANCODE_2) {
                        set_scene(heart_scene);
                    }
                }
            }
        }

        set_memory(audio_samples, audio_sample_count * sizeof(f32), 0);
        current_scene.audio(current_scene.state, audio_samples, audio_sample_count);
        SDL_QueueAudio(audio_device, audio_samples, audio_sample_count * sizeof(f32));

        current_scene.frame(current_scene.state);

        draw_text(debug_font, 250, 160, ~0, "FPS: %.0f", 1.0f / delta_time);
        draw_text(debug_font, 250, 172, ~0, "BUF: %d",
            SDL_GetQueuedAudioSize(audio_device) / sizeof(f32) / 2);

        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, WIDTH * sizeof(pixels[0]));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
