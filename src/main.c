#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#include <sys/mman.h>

#define POOL_STATIC_ALLOCATE
#define POOL_STATIC_PERSIST_BYTE_COUNT (64 * 1000 * 1000)

#include "common.c"
#include "memory.c"
#include "graphics.c"
#include "assets.c"

int main(int argument_count, char ** arguments) {
    setbuf(stdout, 0);

    init_memory_pools(megabytes(64), megabytes(32), megabytes(8));

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture * screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    pixels = pool_alloc(PERSIST_POOL, WIDTH * HEIGHT * sizeof(u32));

    Animated_Image animated_image = {
        .width = 320,
        .height = 200,
        .frame_count = 7,
        .frame_duration_ms = 30,
        .start_time_ms = SDL_GetTicks(),
    };

    {
        Image temp = load_pam(PERSIST_POOL, "../assets/heart.pam");
        SDL_assert(temp.pixels);
        animated_image.pixels = temp.pixels;
    }

    Image font_image = load_pam(PERSIST_POOL, "../assets/font.pam");
    Font font = {
        .pixels = font_image.pixels,
        .char_width  = 6,
        .char_height = 12,
    };

    bool pump = false;
    int presses = 0;

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                print_memory_stats();
                exit(0);
            } else if (event.type == SDL_KEYDOWN) {
                if (!event.key.repeat) {
                    pump = true;
                    animated_image.start_time_ms = SDL_GetTicks();
                }
            } else if (event.type == SDL_KEYUP) {
                pump = false;
                animated_image.start_time_ms = SDL_GetTicks();
                ++presses;
            }
        }

        clear(0);

        if (pump) {
            draw_animated_image_frames_and_wait(animated_image, 3, 5, 0, 0);
        } else {
            draw_animated_image_frames_and_wait(animated_image, 0, 2, 0, 0);
        }

        draw_text(font, 10, 10, "Pumps: %d", presses);

        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, WIDTH * sizeof(pixels[0]));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
