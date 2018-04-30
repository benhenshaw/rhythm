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

void prepare_blank_cut(float time_in_seconds, u32 colour, Scene * next_scene, Sound * end_sound)
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

void blank_cut(float time_in_seconds, u32 colour, Scene * next_scene, Sound * end_sound)
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

void prepare_text_cut(float time_in_seconds, u32 background_colour, u32 text_colour,
    Font * font, char * text, Scene * next_scene, Sound * end_sound)
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
}
Heart_State;

Heart_State heart_state;

void heart_start(void * state)
{
    Heart_State * s = state;
    *s = (Heart_State){};
    s->heart = assets.heart_animation;
    s->heart.frame_duration_ms = 50;
    s->heart.start_time_ms = SDL_GetTicks();
}

void heart_frame(void * state, float delta_time)
{
    Heart_State * s = state;
    clear(0);
    draw_animated_image(s->heart, 0, 0);
}

void heart_input(void * state, int player, bool pressed) {}

Scene heart_scene =
{
    .frame = heart_frame,
    .start = heart_start,
    .input = heart_input,
    .state = &heart_state,
};

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
