//
// scene.c
//
// This file contains:
//     - Scene handling
//     - Scene definitions (e.g. the mini-games)
//

typedef void (* Audio_Func)(void * state, float * samples, int sample_count);
typedef void (* Frame_Func)(void * state);
typedef void (* Start_Func)(void * state);
typedef void (* Input_Func)(void * state, int player, bool pressed);

typedef struct {
    Audio_Func audio;
    Frame_Func frame;
    Start_Func start;
    Input_Func input;
    void * state;
} Scene;

Scene current_scene;

void set_scene(Scene scene) {
    // Clear scene and frame memory pools.
    flush_pool(SCENE_POOL);
    flush_pool(FRAME_POOL);

    // Set function pointers.
    if (scene.audio && scene.frame && scene.start && scene.input && scene.state) {
        current_scene = scene;
    }

    current_scene.start(current_scene.state);
}



//
// Heart scene.
//

typedef struct {
    bool pumping;
    int pump_count;
    int start_ms;
    Animated_Image heart;
    Font font;
} Heart_State;

Heart_State heart_state;

void heart_audio(void * state, float * samples, int sample_count) {
    Heart_State * s = state;
    static float phase = 0.0f;
    if (s->pumping) {
        for (int i = 0; i < sample_count; ++i) {
            samples[i] = sinf(phase) * 0.1f;
            phase += 0.02;
        }
    }
}

void heart_frame(void * state) {
    Heart_State * s = state;

    clear(rgba(80, 30, 30, 255));

    if (s->pumping) {
        draw_animated_image_frames_and_wait(s->heart, 3, 5, 0, 0);
    } else {
        draw_animated_image_frames_and_wait(s->heart, 0, 2, 0, 0);
    }

    draw_text(s->font, 10, 10, ~0, "Pumps: %4d", s->pump_count);

    float minutes_since_start = ((SDL_GetTicks() - s->start_ms) * 0.00001f);
    float pumps_per_minute = (float)s->pump_count / minutes_since_start;

    float distance_from_target = fabs(pumps_per_minute - 90.0) / 20.0;
    distance_from_target = clamp(0.0f, distance_from_target, 1.0f);

    u32 colour = rgba(
        55 + 200.0f * distance_from_target,
        100 + 80 * (1.0f - distance_from_target),
        100,
        255);

    draw_text(s->font, 10, 22, colour, "BPM: %6.1f", pumps_per_minute);
}

void heart_start(void * state) {
    Heart_State * s = state;

    s->pumping = false;
    s->pump_count = 0;
    s->start_ms = SDL_GetTicks();

    s->heart = (Animated_Image){
        .width = 320,
        .height = 200,
        .frame_count = 7,
        .frame_duration_ms = 20,
        .start_time_ms = SDL_GetTicks(),
    };

    // TODO: Should all assets be loaded at launch?
    {
        Image temp = load_pam(SCENE_POOL, "../assets/heart.pam");
        SDL_assert(temp.pixels);
        s->heart.pixels = temp.pixels;
    }

    Image font_image = load_pam(SCENE_POOL, "../assets/font.pam");
    s->font = (Font){
        .pixels = font_image.pixels,
        .char_width  = 6,
        .char_height = 12,
    };
}

void heart_input(void * state, int player, bool pressed) {
    Heart_State * s = state;

    if (pressed) {
        if (player == 0) {
            if (!s->pumping) {
                s->pumping = true;
                s->heart.start_time_ms = SDL_GetTicks();
            }
        } else if (player == 1) {
            if (s->pumping) {
                s->pumping = false;
                s->heart.start_time_ms = SDL_GetTicks();
                ++s->pump_count;
            }
        }
    }
}

Scene heart_scene = {
    .audio = heart_audio,
    .frame = heart_frame,
    .start = heart_start,
    .input = heart_input,
    .state = &heart_state,
};



//
// Menu scene.
//

typedef struct {
    Font font;
} Menu_State;

Menu_State menu_state;

void menu_audio(void * state, float * samples, int sample_count) {

}

void menu_frame(void * state) {
    Menu_State * s = state;

    clear(rgba(200, 100, 100, 255));
    draw_text(s->font, 20, 75,  rgba(200, 200, 200, 255), "Rhythm Game");
    draw_text(s->font, 20, 94,  rgba(200, 200, 200, 255), "PLAY");
    draw_text(s->font, 20, 106, rgba(200, 200, 200, 255), "QUIT");
}

void menu_start(void * state) {
    Menu_State * s = state;

    Image font_image = load_pam(SCENE_POOL, "../assets/font.pam");
    s->font = (Font){
        .pixels = font_image.pixels,
        .char_width  = 6,
        .char_height = 12,
    };
}

void menu_input(void * state, int player, bool pressed) {
    if (pressed) {
        set_scene(heart_scene);
    }
}

Scene menu_scene = {
    .audio = menu_audio,
    .frame = menu_frame,
    .start = menu_start,
    .input = menu_input,
    .state = &menu_state,
};
