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
extern Scene menu_scene;

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
//

typedef struct
{
    u32 end_time;
    u32 colour;
    float time_in_seconds;
    Scene * next_scene;
    Sound end_sound;
}
Blank_State;

Blank_State blank_state;

void blank_frame(void * state, float delta_time)
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

void blank_input(void * state, int player, bool pressed) {}

Scene blank_scene =
{
    .start = blank_start,
    .frame = blank_frame,
    .input = blank_input,
    .state = &blank_state,
};

void prepare_blank_cut(float time_in_seconds, u32 colour,
    Scene * next_scene, Sound * end_sound)
{
    blank_state = (Blank_State)
    {
        .time_in_seconds = time_in_seconds,
        .next_scene = next_scene,
        .colour = colour,
        .end_sound = *end_sound,
    };

    set_scene(blank_scene);
}

void blank_cut(float time_in_seconds, u32 colour,
    Scene * next_scene, Sound * end_sound)
{
    prepare_blank_cut(time_in_seconds, colour, next_scene, end_sound);
    set_scene(blank_scene);
}

//
// Text scene.
//
// Display some text on the screen for a given amount of time.
//

typedef struct
{
    u32 end_time;
    u32 background_colour;
    u32 text_colour;
    int x;
    int y;
    float time_in_seconds;
    Scene * next_scene;
    Sound end_sound;
    char * text;
    Font font;
}
Text_State;

Text_State text_state;

void text_frame(void * state, float delta_time)
{
    Text_State * s = state;
    if (s->end_time < SDL_GetTicks())
    {
        if (s->end_sound.samples)
        {
            play_sound(&mixer, s->end_sound, 1.0, 1.0, false);
        }
        set_scene(*s->next_scene);
    }
    clear(s->background_colour);
    draw_text(s->font, s->x, s->y, s->text_colour, s->text);
}

void text_start(void * state)
{
    Text_State * s = state;
    s->end_time = SDL_GetTicks() + (1000 * s->time_in_seconds);
}

void text_input(void * state, int player, bool pressed) {}


Scene text_scene =
{
    .start = text_start,
    .frame = text_frame,
    .input = text_input,
    .state = &text_state,
};

void prepare_text_cut(float time_in_seconds,
    u32 background_colour, u32 text_colour,
    Font * font, char * text,
    Scene * next_scene, Sound * end_sound)
{
    int string_length = strlen(text);
    int x = (WIDTH / 2) - (font->char_width * string_length) / 2;
    int y = (HEIGHT / 2) - (font->char_height / 2);

    text_state = (Text_State)
    {
        .time_in_seconds = time_in_seconds,
        .next_scene = next_scene,
        .background_colour = background_colour,
        .end_sound = *end_sound,
        .text = text,
        .text_colour = text_colour,
        .font = *font,
        .x = x,
        .y = y,
    };
}

void text_cut(float time_in_seconds, u32 background_colour, u32 text_colour,
    Font * font, char * text, Scene * next_scene, Sound * end_sound)
{
    prepare_text_cut(time_in_seconds,
        background_colour, text_colour, font, text,
        next_scene, end_sound);
    set_scene(text_scene);
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
    float target_beats_per_minute;
    float accuracy;
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
    s->draw_interface = true;
    play_sound(&mixer, assets.wood_block_sound, 1.0, 1.0, true);
}

void heart_frame(void * state, float delta_time)
{
    Heart_State * s = state;

    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            int r = random_int_range(0, 40);
            pixels[x + y * WIDTH] = rgba(r,r,r,255);
        }
    }

    if (s->expanding)
    {
        draw_animated_image_frames_and_wait(s->heart, 0, 3, 0, 20);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->heart, 4, 6, 0, 20);
    }


    float green_range = 5.0;
    float yellow_range = green_range * 5.0;
    float red_range = green_range * 10.0f;
    float scale = 100.0 / red_range;

    if (s->time_stamps[0] && s->time_stamps[1])
    {
        float beats_per_minute = 60.0f / (s->delta_ms * 0.001f);
        float d = s->target_beats_per_minute - beats_per_minute;
        d = clamp(-red_range, -d, red_range);
        s->accuracy += (d - s->accuracy) * 0.05;
    }

    if (s->draw_interface)
    {
        int y = 10;
        draw_line(WIDTH / 2 - red_range * scale, HEIGHT - y,
            WIDTH / 2 + red_range * scale, HEIGHT - y,
            0xff0000ff);
        draw_line(WIDTH / 2 - yellow_range * scale, HEIGHT - y,
            WIDTH / 2 + yellow_range * scale, HEIGHT - y,
            0xffff00ff);
        draw_line(WIDTH / 2 - green_range * scale, HEIGHT - y,
            WIDTH / 2 + green_range * scale, HEIGHT - y,
            0x00ff00ff);
        draw_line(WIDTH / 2 - green_range * scale, HEIGHT - (y - 1),
            WIDTH / 2 - green_range * scale, HEIGHT - (y + 1),
            0x00ff00ff);
        draw_line(WIDTH / 2 + green_range * scale, HEIGHT - (y - 1),
            WIDTH / 2 + green_range * scale, HEIGHT - (y + 1),
            0x00ff00ff);

        draw_line(WIDTH / 2 + s->accuracy * scale, HEIGHT - (y - 1),
            WIDTH / 2 + s->accuracy * scale, HEIGHT - (y + 1),
            ~0);
        draw_line(WIDTH / 2 + s->accuracy * scale - 1, HEIGHT - (y - 1),
            WIDTH / 2 + s->accuracy * scale - 1, HEIGHT - (y + 1),
            ~0);
        draw_line(WIDTH / 2 + s->accuracy * scale + 1, HEIGHT - (y - 1),
            WIDTH / 2 + s->accuracy * scale + 1, HEIGHT - (y + 1),
            ~0);

        draw_text(assets.main_font, WIDTH / 2 - red_range * scale - 30, HEIGHT - (y + 6), ~0, "slow");
        draw_text(assets.main_font, WIDTH / 2 + red_range * scale + 5, HEIGHT - (y + 6), ~0, "fast");

        y = 80 + sinf((M_PI*2.0) * SDL_GetTicks() * 0.001 * (s->target_beats_per_minute / 60.0)) * 5;
        if (s->expanding)
        {
            draw_line(49, y, 49, y + 10, ~0);
            draw_line(49, y + 10, 49 - 5, y + 5, ~0);
            draw_line(49, y + 10, 49 + 5, y + 5, ~0);
        }
        else
        {
            draw_line(269, y, 269, y + 10, ~0);
            draw_line(269, y + 10, 269 - 5, y + 5, ~0);
            draw_line(269, y + 10, 269 + 5, y + 5, ~0);
        }

        draw_animated_image_frame(assets.button_animation, s->player_states[0], 20, 110);
        draw_animated_image_frame(assets.button_animation, s->player_states[1], 240, 110);

    }
}

void heart_input(void * state, int player, bool pressed)
{
    Heart_State * s = state;
    s->player_states[player] = pressed;
    if (pressed)
    {
        if (s->expanding != player)
        {
            int t = SDL_GetTicks();
            s->time_stamps[player] = t;
            s->heart.start_time_ms = t;
            int a = s->time_stamps[0];
            int b = s->time_stamps[1];
            if (a && b)
            {
                s->delta_ms = abs(a - b);
            }
            s->expanding = player;
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

#undef STAMP_COUNT

//
// Morse scene.
//

typedef struct
{
    Image background;
}
Morse_State;

Morse_State morse_state;

void morse_start(void * state)
{
    Morse_State * s = state;
    s->background = read_image_file(SCENE_POOL, "../assets/morse.pam");
    SDL_assert(s->background.pixels);
}

void morse_frame(void * state, float delta_time)
{
    Morse_State * s = state;
    draw_image(s->background, 0, 0);
}

void morse_input(void * state, int player, bool pressed)
{

}

Scene morse_scene =
{
    .start = morse_start,
    .frame = morse_frame,
    .input = morse_input,
    .state = &morse_state,
};

//
// Menu scene.
//

typedef struct
{
    Font font;
}
Menu_State;

Menu_State menu_state;

void menu_frame(void * state, float delta_time)
{
    Menu_State * s = state;

    clear(rgba(200, 100, 100, 255));
    draw_text(s->font, 20, 75,  rgba(200, 200, 200, 255), "Rhythm Game");
    draw_text(s->font, 20, 94,  rgba(200, 200, 200, 255), "PLAY");
    draw_text(s->font, 20, 106, rgba(200, 200, 200, 255), "QUIT");
}

void menu_start(void * state)
{
    Menu_State * s = state;
    *s = (Menu_State){};
    s->font = assets.main_font;
}

void menu_input(void * state, int player, bool pressed)
{
}

Scene menu_scene =
{
    .frame = menu_frame,
    .start = menu_start,
    .input = menu_input,
    .state = &menu_state,
};
