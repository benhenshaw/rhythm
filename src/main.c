//
// main.c
//
// This file contains:
//     - Program entry point
//     - Initialisation for graphics and audio.
//     - Frame loop.
//     - Main audio callback.
//

// Compile time options for the memory allocator.
#define POOL_STATIC_ALLOCATE
#define POOL_STATIC_PERSIST_BYTE_COUNT (64 * 1000 * 1000)

// External includes here:
#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>

// The entire project is a single compilation unit.
// Everything is included here:
#include "common.c"
#include "memory.c"
#include "graphics.c"
#include "audio.c"
#include "assets.c"
#include "scene.c"

//
// Main audio callback.
//

void audio_callback(void * data, u8 * stream, int byte_count) {
    Mixer * mixer = data;
    f32 * samples = (f32 *)stream;
    int sample_count = byte_count / sizeof(f32);
    mix_audio(mixer, samples, sample_count);
}

//
// Program entry point.
//

int main(int argument_count, char ** arguments) {
    // DEBUG: Unbuffered logging.
    setbuf(stdout, 0);

    //
    // Initialisation.
    //

    if (!init_memory_pools(megabytes(64), megabytes(32), megabytes(8))) {
        panic_exit("Could not initialise memory pools.");
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        panic_exit("Could not initialise SDL2.\n%s", SDL_GetError());
    }

    //
    // Init graphics.
    //

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);
    if (!window) {
        panic_exit("Could not create a window.\n%s", SDL_GetError());
    }

    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        panic_exit("Could not create a rendering context.\n%s", SDL_GetError());
    }

    SDL_Texture * screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);
    if (!screen_texture) {
        panic_exit("Could not create the screen texture.\n%s", SDL_GetError());
    }

    pixels = pool_alloc(PERSIST_POOL, WIDTH * HEIGHT * sizeof(u32));

    SDL_ShowCursor(false);

    //
    // Init audio.
    //

    mixer = create_mixer(PERSIST_POOL, 64, 1.0);

    SDL_AudioSpec audio_output_spec = {
        .freq = 48000,
        .format = AUDIO_F32,
        .channels = 2,
        .samples = 64,
        .callback = audio_callback,
        .userdata = &mixer,
    };

    audio_device = SDL_OpenAudioDevice(NULL, false,
        &audio_output_spec, NULL, 0);
    if (!audio_device) {
        panic_exit("Could not open the audio device.\n%s", SDL_GetError());
    }

    SDL_PauseAudioDevice(audio_device, false);

    //
    // Set up the timers.
    //

    u64 previous_counter_ticks = SDL_GetPerformanceCounter();
    f32 counter_ticks_per_second = SDL_GetPerformanceFrequency();

    //
    // DEBUG:
    //

    Image font_image = read_image_file(PERSIST_POOL, "../assets/font.pam");
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
        // Update timers.
        f32 delta_time = (SDL_GetPerformanceCounter() - previous_counter_ticks)
                            / counter_ticks_per_second;
        previous_counter_ticks = SDL_GetPerformanceCounter();

        // Handle events since last frame.
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

                    // DEBUG: scene switching.
                    else if (sc == SDL_SCANCODE_1) {
                        set_scene(menu_scene);
                    } else if (sc == SDL_SCANCODE_2) {
                        set_scene(heart_scene);
                    }
                }
            }
        }

        // Render the scene.
        current_scene.frame(current_scene.state, delta_time);

        // DEBUG:
        draw_text(debug_font, 270, 182, ~0, "FPS: %.0f", 1.0f / delta_time);

        // Render the internal pixel buffer to the screen.
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, WIDTH * sizeof(pixels[0]));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
