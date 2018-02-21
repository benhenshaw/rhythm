#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#include <sys/mman.h>

#define POOL_STATIC_ALLOCATE
#define POOL_STATIC_PERSIST_BYTE_COUNT (64 * 1000 * 1000)

#include "common.c"
#include "memory.c"
#include "graphics.c"
#include "assets.c"
#include "scene.c"

int main(int argument_count, char ** arguments) {
    setbuf(stdout, 0);

    init_memory_pools(megabytes(64), megabytes(32), megabytes(8));

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);

    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture * screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    pixels = pool_alloc(PERSIST_POOL, WIDTH * HEIGHT * sizeof(u32));

    set_scene(heart_scene);

    while (true) {
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

                    // DEBUG

                    else if (sc == SDL_SCANCODE_1) {
                        set_scene(menu_scene);
                    } else if (sc == SDL_SCANCODE_2) {
                        set_scene(heart_scene);
                    }
                }
            }
        }

        current_scene.frame(current_scene.state);

        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, WIDTH * sizeof(pixels[0]));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
