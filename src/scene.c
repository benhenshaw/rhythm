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
void blank_start(void * state) {}
void blank_input(void * state, int player, bool pressed) {}


Scene blank_scene =
{
    .start = blank_start,
    .frame = blank_frame,
    .input = blank_input,
    .state = &blank_state,
};

void blank_cut(float time_in_seconds, u32 colour, Scene * next_scene, Sound * end_sound)
{
    set_scene(blank_scene);
    blank_state.end_time = SDL_GetTicks() + (1000 * time_in_seconds);
    blank_state.next_scene = next_scene;
    blank_state.colour = colour;
    blank_state.end_sound = *end_sound;
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

void heart_input(void * state, int player, bool pressed)
{
    Heart_State * s = state;
}

Scene heart_scene =
{
    .frame = heart_frame,
    .start = heart_start,
    .input = heart_input,
    .state = &heart_state,
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
