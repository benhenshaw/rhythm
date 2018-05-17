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

typedef void (* Frame_Func)(void * state, f32 delta_time);
typedef void (* Start_Func)(void * state);
typedef void (* Input_Func)(void * state, int player, bool pressed, u32 time_stamp_ms);

typedef struct
{
    Start_Func start;
    Frame_Func frame;
    Input_Func input;
    void * state;
}
Scene;

// TODO: Where should the scenes live?
Scene current_scene;
extern Scene heart_scene;
extern Scene lungs_scene;
extern Scene digestion_scene;


// Change the current scene.
// Will call the start function for that scene.
// Returns true if successful.
bool set_scene(Scene scene)
{
    // Clear scene and frame memory pools.
    flush_pool(SCENE_POOL);
    flush_pool(FRAME_POOL);
    // Set function pointers.
    if (scene.start && scene.frame && scene.input && scene.state)
    {
        current_scene = scene;
        // Call the start function for the new scene.
        current_scene.start(current_scene.state);
        return true;
    }
    return false;
}

//
// Blank scene.
//
// Display a blank screen of a given colour, for a given amount of time.
// Can be used for transitions between scenes.
//

typedef struct
{
    u32 end_time;
    u32 colour;
    f32 time_in_seconds;
    Scene * next_scene;
    Sound end_sound;
}
Blank_State;

Blank_State blank_state;

void blank_frame(void * state, f32 delta_time)
{
    Blank_State * s = state;
    if (s->end_time < SDL_GetTicks())
    {
        if (s->end_sound.samples)
        {
            play_sound(&mixer, s->end_sound, 1.0, 1.0, false);
        }
        set_scene(*s->next_scene);
    }
    clear(s->colour);
}

// Stub functions, as nothing needs to be done for this scene.
void blank_start(void * state)
{
    Blank_State * s = state;
    s->end_time = SDL_GetTicks() + (1000 * s->time_in_seconds);
}

void blank_input(void * state, int player, bool pressed, u32 time_stamp_ms) {}

Scene blank_scene =
{
    .start = blank_start,
    .frame = blank_frame,
    .input = blank_input,
    .state = &blank_state,
};

void prepare_blank_cut(f32 time_in_seconds, u32 colour,
    Scene * next_scene, Sound * end_sound)
{
    blank_state = (Blank_State)
    {
        .time_in_seconds = time_in_seconds,
        .next_scene = next_scene,
        .colour = colour,
        .end_sound = end_sound ? *end_sound : (Sound){},
    };

    set_scene(blank_scene);
}

void blank_cut(f32 time_in_seconds, u32 colour,
    Scene * next_scene, Sound * end_sound)
{
    prepare_blank_cut(time_in_seconds, colour, next_scene, end_sound);
    set_scene(blank_scene);
}

//
// Accuracy interface.
//
// A tutorial interface which helps players improve their accuracy.
//
void draw_accuracy_interface(f32 accuracy,
    f32 range, f32 bpm,
    bool draw_left_arrow, bool draw_right_arrow,
    bool left_state, bool right_state)
{
    f32 yellow_range = range * 5.0;
    f32 red_range = range * 10.0f;
    f32 scale = 100.0 / red_range;

    int y = 10;
    draw_line(WIDTH / 2 - red_range * scale, HEIGHT - y,
        WIDTH / 2 + red_range * scale, HEIGHT - y,
        0xff0000ff);
    draw_line(WIDTH / 2 - yellow_range * scale, HEIGHT - y,
        WIDTH / 2 + yellow_range * scale, HEIGHT - y,
        0xffff00ff);
    draw_line(WIDTH / 2 - range * scale, HEIGHT - y,
        WIDTH / 2 + range * scale, HEIGHT - y,
        0x00ff00ff);
    draw_line(WIDTH / 2 - range * scale, HEIGHT - (y - 1),
        WIDTH / 2 - range * scale, HEIGHT - (y + 1),
        0x00ff00ff);
    draw_line(WIDTH / 2 + range * scale, HEIGHT - (y - 1),
        WIDTH / 2 + range * scale, HEIGHT - (y + 1),
        0x00ff00ff);

    draw_line(WIDTH / 2 + accuracy * scale, HEIGHT - (y - 1),
        WIDTH / 2 + accuracy * scale, HEIGHT - (y + 1),
        ~0);
    draw_line(WIDTH / 2 + accuracy * scale - 1, HEIGHT - (y - 1),
        WIDTH / 2 + accuracy * scale - 1, HEIGHT - (y + 1),
        ~0);
    draw_line(WIDTH / 2 + accuracy * scale + 1, HEIGHT - (y - 1),
        WIDTH / 2 + accuracy * scale + 1, HEIGHT - (y + 1),
        ~0);

    draw_text(assets.main_font,
        WIDTH / 2 - red_range * scale - 30,
        HEIGHT - (y + 6),
        ~0,
        "slow");
    draw_text(assets.main_font,
        WIDTH / 2 + red_range * scale + 5,
        HEIGHT - (y + 6),
        ~0,
        "fast");

    y = 80 + sinf((M_PI*2.0) * SDL_GetTicks() * 0.001 * (bpm / 60.0)) * 5;
    if (draw_left_arrow)
    {
        draw_line(44, y,      44,     y + 10, ~0);
        draw_line(44, y + 10, 44 - 5, y +  5, ~0);
        draw_line(44, y + 10, 44 + 5, y +  5, ~0);
    }

    if (draw_right_arrow)
    {
        draw_line(274, y,      274,     y + 10, ~0);
        draw_line(274, y + 10, 274 - 5, y +  5, ~0);
        draw_line(274, y + 10, 274 + 5, y +  5, ~0);
    }

    draw_animated_image_frame(assets.button_animation, left_state,   15, 110);
    draw_animated_image_frame(assets.button_animation, right_state, 245, 110);
}

//
// Heart scene.
//
// In this scene, players will need to work together to get a heart pumping at
// a resonable rate. One player controls expansion of the heart, while the other
// controls contraction.
//

typedef struct
{
    Animated_Image heart;
    bool player_states[2];
    int time_stamps[2];
    int delta_ms;
    f32 target_beats_per_minute;
    f32 accuracy;
    f32 accuracy_timer;
    f32 target_accuracy_time;
    bool expanding;
    bool draw_interface;
}
Heart_State;

Heart_State heart_state;

void heart_start(void * state)
{
    Heart_State * s = state;
    *s = (Heart_State){};
    s->heart = assets.heart_animation;
    s->heart.frame_duration_ms = 30;
    s->target_beats_per_minute = 60.0;
    s->accuracy = -50.0;
    s->target_accuracy_time = 10.0;
    if (!sound_is_playing(&mixer, assets.brown_sound))
    {
        play_sound(&mixer, assets.brown_sound, 0.05, 0.05, true);
    }
}

void heart_frame(void * state, f32 delta_time)
{
    Heart_State * s = state;

    draw_noise(s->accuracy_timer * 0.5);

    if (s->expanding)
    {
        draw_animated_image_frames_and_wait(s->heart, 0, 3, 0, 20);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->heart, 4, 6, 0, 20);
    }

    f32 range = 5.0;
    if (s->time_stamps[0] && s->time_stamps[1])
    {
        f32 beats_per_minute = 60.0f / (s->delta_ms * 0.001f);
        f32 d = s->target_beats_per_minute - beats_per_minute;
        d = clamp(-range * 10.0, -d, range * 10.0);
        s->accuracy += (d - s->accuracy) * 0.05;
    }

    s->accuracy_timer += delta_time / s->target_accuracy_time;
    if (fabsf(s->accuracy) > range) s->accuracy_timer = 0.0;
    s->accuracy -= delta_time;

    if (s->draw_interface)
    {
        draw_accuracy_interface(s->accuracy, range,
            s->target_beats_per_minute,
            s->expanding, !s->expanding,
            s->player_states[0], s->player_states[1]);
    }
}

void heart_input(void * state, int player, bool pressed, u32 time_stamp_ms)
{
    Heart_State * s = state;
    s->player_states[player] = pressed;
    if (pressed)
    {
        play_sound(&mixer, assets.wood_block_sound,
            player ? 0.1 : 1.0,
            player ? 1.0 : 0.1,
            false);
        if (s->expanding != player)
        {
            s->time_stamps[player] = time_stamp_ms;
            s->heart.start_time_ms = time_stamp_ms;
            int a = s->time_stamps[0];
            int b = s->time_stamps[1];
            if (a && b) s->delta_ms = abs(a - b);
            s->expanding = player;
        }

        if (s->accuracy_timer > 1.0)
        {
            stop_sound(&mixer, assets.brown_sound);
            blank_cut(3.0, 0, &lungs_scene, NULL);
        }
    }
}

Scene heart_scene =
{
    .frame = heart_frame,
    .start = heart_start,
    .input = heart_input,
    .state = &heart_state,
};

//
// Lungs scene.
//
// In this scene, players hold and release their buttons in time with
// eachother and the beat.
//

typedef struct
{
    Animated_Image left_lung;
    Animated_Image right_lung;
    f32 target_beats_per_minute;
    f32 accuracy;
    f32 accuracy_timer;
    f32 target_accuracy_time;
    int delta_ms[2];
    int time_stamps[2][2];
    int current_stamp[2];
    bool player_states[2];
    bool draw_interface;
}
Lungs_State;

Lungs_State lungs_state;

void lungs_start(void * state)
{
    Lungs_State * s = state;
    *s = (Lungs_State){};
    s->target_beats_per_minute = 60.0;
    s->left_lung = assets.left_lung_animation;
    s->right_lung = assets.right_lung_animation;
    s->left_lung.frame_duration_ms = 60.0;
    s->right_lung.frame_duration_ms = 60.0;
    s->target_accuracy_time = 10.0;
    s->accuracy = -50.0;
    if (!sound_is_playing(&mixer, assets.brown_sound))
    {
        play_sound(&mixer, assets.brown_sound, 0.05, 0.05, true);
    }
}

void lungs_frame(void * state, f32 delta_time)
{
    Lungs_State * s = state;

    draw_noise(s->accuracy_timer * 0.5);

    if (s->player_states[0])
    {
        draw_animated_image_frames_and_wait(s->left_lung, 0, 3, 76, 40);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->left_lung, 5, 7, 76, 40);
    }

    if (s->player_states[1])
    {
        draw_animated_image_frames_and_wait(s->right_lung, 0, 3, 76 + 75, 40);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->right_lung, 5, 7, 76 + 75, 40);
    }

    f32 range = 5.0;
    if (s->time_stamps[0] && s->time_stamps[1])
    {
        f32 beats_per_minute[2];
        beats_per_minute[0] = 60.0f / (s->delta_ms[0] * 0.001f);
        beats_per_minute[1] = 60.0f / (s->delta_ms[1] * 0.001f);
        f32 target_delta = s->target_beats_per_minute - beats_per_minute[0];
        target_delta += s->target_beats_per_minute - beats_per_minute[1];
        target_delta /= 2.0;
        target_delta = clamp(-range * 10.0, -target_delta, range * 10.0);
        s->accuracy += (target_delta - s->accuracy) * 0.05;
    }

    s->accuracy_timer += delta_time / s->target_accuracy_time;
    if (fabsf(s->accuracy) > range) s->accuracy_timer = 0.0;
    s->accuracy -= delta_time;

    if (s->draw_interface)
    {
        draw_accuracy_interface(s->accuracy, range,
            s->target_beats_per_minute,
            true, true,
            s->player_states[0], s->player_states[1]);
    }
}

void lungs_input(void * state, int player, bool pressed, u32 time_stamp_ms)
{
    Lungs_State * s = state;
    s->player_states[player] = pressed;
    s->time_stamps[player][s->current_stamp[player]] = time_stamp_ms;
    int a = s->time_stamps[player][0];
    int b = s->time_stamps[player][1];
    if (a && b) s->delta_ms[player] = abs(a - b);

    s->current_stamp[player] = !s->current_stamp[player];

    if (player == 0)
    {
        s->left_lung.start_time_ms = SDL_GetTicks();
        play_sound(&mixer, assets.shaker_sound, 0.4, 0.04, false);
    }
    else
    {
        s->right_lung.start_time_ms = SDL_GetTicks();
        play_sound(&mixer, assets.shaker_sound, 0.04, 0.4, false);
    }

    if (s->accuracy_timer > 1.0)
    {
        stop_sound(&mixer, assets.brown_sound);
        blank_cut(3.0, 0, &digestion_scene, NULL);
    }
}

Scene lungs_scene =
{
    .frame = lungs_frame,
    .start = lungs_start,
    .input = lungs_input,
    .state = &lungs_state,
};

//
// Digestion scene.
//
// In this scene players need to tap out a rhythm in 5/4 with one player
// tapping on the final beat of the bar, and one tapping the rest.
//

typedef struct
{
    Animated_Image digestion;
    int current_beat;
    f32 accuracy;
    f32 accuracy_timer;
    f32 target_accuracy_time;
    f32 target_beats_per_minute;
    int last_press_time_ms;
    bool draw_interface;
    bool player_states[2];

}
Digestion_State;

Digestion_State digestion_state;

void digestion_start(void * state)
{
    Digestion_State * s = state;
    *s = (Digestion_State){};
    s->digestion = assets.digestion_animation;
    s->digestion.frame_duration_ms = 30;
    s->target_beats_per_minute = 60;
    s->accuracy = -50.0;
    s->target_accuracy_time = 10.0;
    if (!sound_is_playing(&mixer, assets.brown_sound))
    {
        play_sound(&mixer, assets.brown_sound, 0.05, 0.05, true);
    }
}

void digestion_frame(void * state, f32 delta_time)
{
    Digestion_State * s = state;
    draw_noise(s->accuracy_timer * 0.5);

    if (s->current_beat == 5)
    {
        bool waiting = draw_animated_image_frames_and_wait(s->digestion, 4, 6, 117, 40);
        if (waiting) s->current_beat = 0;
    }
    else
    {
        draw_animated_image_frame(s->digestion, s->current_beat, 117, 40);
    }

    f32 range = 5.0;
    s->accuracy_timer += delta_time / s->target_accuracy_time;
    if (fabsf(s->accuracy) > range) s->accuracy_timer = 0.0;
    s->accuracy -= delta_time;

    if (s->draw_interface)
    {
        draw_accuracy_interface(s->accuracy,
            range, s->target_beats_per_minute,
            s->current_beat < 4, s->current_beat == 4,
            s->player_states[0], s->player_states[1]);
    }
}

void digestion_input(void * state, int player, bool pressed, u32 time_stamp_ms)
{
    Digestion_State * s = state;
    s->player_states[player] = pressed;
    if (pressed)
    {
        s->accuracy = (time_stamp_ms - s->last_press_time_ms) * 0.001;
        if ((s->current_beat < 4 && player == 0) || (s->current_beat == 4 && player == 1))
        {
            s->current_beat = (s->current_beat + 1) % 6;
            s->digestion.start_time_ms = time_stamp_ms;
            if (s->current_beat == 5)
            {
                play_sound(&mixer, assets.wood_block_sound, 0.3, 1.0, false);
            }
            else
            {
                play_sound(&mixer, assets.tap_sound, 1.0, 0.3, false);
            }
        }
        s->last_press_time_ms = time_stamp_ms;

        if (s->accuracy_timer > 1.0)
        {
            stop_sound(&mixer, assets.brown_sound);
            blank_cut(3.0, 0, &heart_scene, NULL);
        }
    }
}

Scene digestion_scene =
{
    .frame = digestion_frame,
    .start = digestion_start,
    .input = digestion_input,
    .state = &digestion_state,
};
