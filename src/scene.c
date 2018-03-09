//
// scene.c
//
// This file contains:
//     - Scene handling.
//     - Scene definitions (e.g. the mini-games).
//

//
// Scene.
//
// A scene is a set of functions and a struct of state variables that can be
// swapped out at will. The start function is called when the scene is entered,
// the frame function is called when the frame needs to be redrawn, and the
// input function is called when a player presses a button.
//

typedef void (* Frame_Func)(void * state, float delta_time);
typedef void (* Start_Func)(void * state);
typedef void (* Input_Func)(void * state, int player, bool pressed);

typedef struct {
    Start_Func start;
    Frame_Func frame;
    Input_Func input;
    void * state;
} Scene;

// TODO: Where should the scenes live?
Scene current_scene;
extern Scene heart_scene;
extern Scene menu_scene;

// Change the current scene.
// Will call the start function for that scene.
// Returns true if successful.
bool set_scene(Scene scene) {
    // Clear scene and frame memory pools.
    flush_pool(SCENE_POOL);
    flush_pool(FRAME_POOL);
    // Set function pointers.
    if (scene.start && scene.frame && scene.input && scene.state) {
        current_scene = scene;
        // Call the start function for the new scene.
        current_scene.start(current_scene.state);
        return true;
    }
    return false;
}

//
// Heart scene.
//
// In this scene, players will need to work together to get a heart pumping at
// a resonable rate. One player controls expansion of the heart, while the other
// controls contraction.
//

typedef struct {
    bool complete;
    bool pumping;
    int pump_count;
    int start_ms;
    int previous_beat_time_ms;
    int most_recent_beat_time_ms;
    int press_index;
    float beats_per_minute;
    float score;
    Animated_Image heart;
    Font font;
    Sound sound;
    Sound yay;
    int yay_channel;
} Heart_State;

Heart_State heart_state;

void heart_start(void * state) {
    Heart_State * s = state;

    s->pumping = false;
    s->pump_count = 0;
    s->start_ms = SDL_GetTicks();
    s->most_recent_beat_time_ms = 1;
    s->previous_beat_time_ms = 1;
    s->score = 0.0f;

    s->heart = (Animated_Image){
        .width = 320,
        .height = 200,
        .frame_count = 7,
        .frame_duration_ms = 30,
        .start_time_ms = SDL_GetTicks(),
    };

    // TODO: Should all assets be loaded at launch?
    {
        Image temp = read_image_file(SCENE_POOL, "../assets/heart.pam");
        SDL_assert(temp.pixels);
        s->heart.pixels = temp.pixels;
    }

    Image font_image = read_image_file(SCENE_POOL, "../assets/font.pam");
    s->font = (Font){
        .pixels = font_image.pixels,
        .char_width  = 6,
        .char_height = 12,
    };

    // TODO: Use custom sound file loader!
    {
        SDL_AudioSpec spec = {};
        u8 * samples = 0;
        u32 byte_count = 0;
        SDL_LoadWAV("../assets/woodblock.wav", &spec, &samples, &byte_count);
        SDL_assert(samples);
        s->sound.samples = (f32 * )samples;
        s->sound.sample_count = byte_count / sizeof(f32);
    }

    {
        SDL_AudioSpec spec = {};
        u8 * samples = 0;
        u32 byte_count = 0;
        SDL_LoadWAV("../assets/yay.wav", &spec, &samples, &byte_count);
        SDL_assert(samples);
        s->yay.samples = (f32 * )samples;
        s->yay.sample_count = byte_count / sizeof(f32);
    }

    SDL_LockAudioDevice(audio_device);
    s->yay_channel = queue_sound(&mixer, s->yay, 0.5f, 0.5f, false);
    SDL_UnlockAudioDevice(audio_device);
}

void heart_frame(void * state, float delta_time) {
    Heart_State * s = state;

    if (!s->complete) {
        float bmp_target = 90.0f;
        float allowance = 20.0f;
        float distance_from_target = fabs(s->beats_per_minute - bmp_target);
        float error = clamp(0.0f, distance_from_target / allowance, 1.0f);
        clear(rgba(
            clamp(0, error * 100, 255),
            clamp(0, error * 30, 255),
            clamp(0, error * 30, 255),
            255));

        if (s->pumping) {
            draw_animated_image_frames_and_wait(s->heart, 3, 6, 0, 0);
        } else {
            draw_animated_image_frames_and_wait(s->heart, 0, 3, 0, 0);
        }

        if (distance_from_target < allowance) {
            s->score += delta_time;
        } else {
            s->score = 0;
        }

        if (s->score > 5) {
            s->complete = true;
        }

        if (s->beats_per_minute > 1.0f) s->beats_per_minute *= 0.99f;

        int delta = s->most_recent_beat_time_ms - s->previous_beat_time_ms;
        int since = SDL_GetTicks() - s->most_recent_beat_time_ms;
        if (delta && since < 500) {
            float target = 60000.0f / delta;
            s->beats_per_minute += (target - s->beats_per_minute) * 0.1f;
        }
    } else {
        SDL_LockAudioDevice(audio_device);
        play_channel(&mixer, s->yay_channel);
        SDL_UnlockAudioDevice(audio_device);

        clear(rgba(80, 30, 30, 255));
        s->heart.frame_duration_ms = 60;
        draw_animated_image(s->heart, 0, 0);
    }
}

void heart_input(void * state, int player, bool pressed) {
    Heart_State * s = state;

    if (pressed) {
        if (player == 0) {
            if (!s->pumping) {
                s->pumping = true;
                s->heart.start_time_ms = SDL_GetTicks();
                SDL_LockAudioDevice(audio_device);
                play_sound(&mixer, s->sound, 1.0f, 0.0f, false);
                SDL_UnlockAudioDevice(audio_device);
            }
        } else if (player == 1) {
            if (s->pumping) {
                s->pumping = false;
                int t = SDL_GetTicks();
                s->heart.start_time_ms = t;
                s->previous_beat_time_ms = s->most_recent_beat_time_ms;
                s->most_recent_beat_time_ms = t;
                SDL_LockAudioDevice(audio_device);
                play_sound(&mixer, s->sound, 0.0f, 1.0f, false);
                SDL_UnlockAudioDevice(audio_device);
                SDL_UnlockAudioDevice(audio_device);
            }
        }
    }
}

Scene heart_scene = {
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

void menu_frame(void * state, float delta_time) {
    Menu_State * s = state;

    clear(rgba(200, 100, 100, 255));
    draw_text(s->font, 20, 75,  rgba(200, 200, 200, 255), "Rhythm Game");
    draw_text(s->font, 20, 94,  rgba(200, 200, 200, 255), "PLAY");
    draw_text(s->font, 20, 106, rgba(200, 200, 200, 255), "QUIT");
}

void menu_start(void * state) {
    Menu_State * s = state;

    Image font_image = read_image_file(SCENE_POOL, "../assets/font.pam");
    s->font = (Font){
        .pixels = font_image.pixels,
        .char_width  = 6,
        .char_height = 12,
    };
}

void menu_input(void * state, int player, bool pressed) {
}

Scene menu_scene = {
    .frame = menu_frame,
    .start = menu_start,
    .input = menu_input,
    .state = &menu_state,
};
