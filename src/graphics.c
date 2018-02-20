#define WIDTH 320
#define HEIGHT 200

u32 * pixels;

static inline u32 rgba(u8 r, u8 g, u8 b, u8 a) {
    return (r << 24u) | (g << 16u) | (b << 8u) | a;
}

static inline u32 get_alpha(u32 colour) {
    return (colour & 0x000000ff);
}

void clear(u32 colour) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pixels[x + y * WIDTH] = colour;
        }
    }
}

// Returns false if the given coordinates are off screen.
static inline bool set_pixel(int x, int y, u32 colour) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        pixels[x + y * WIDTH] = colour;
        return true;
    }
    return false;
}

// Draw a line using Bresenham's line algorithm.
void draw_line(int ax, int ay, int bx, int by, u32 colour) {
    int delta_x = abs(bx - ax);
    int delta_y = abs(by - ay);
    int step_x  = ax < bx ? 1 : -1;
    int step_y  = ay < by ? 1 : -1;
    int error   = (delta_x > delta_y ? delta_x : -delta_y) / 2;

    // Stop drawing if pixel is off screen or we have reached the end of the line.
    while (set_pixel(ax, ay, colour) && !(ax == bx && ay == by)) {
        int e = error;
        if (e > -delta_x) error -= delta_y, ax += step_x;
        if (e <  delta_y) error += delta_x, ay += step_y;
    }
}

typedef struct {
    u32 * pixels;
    int width;
    int height;
} Image;

// Draw a bitmap image to the screen.
void draw_image(Image image, int x, int y) {
    int max_x = min(x + image.width, WIDTH);
    int max_y = min(y + image.height, HEIGHT);
    for (int sy = y, iy = 0; sy < max_y; ++sy, ++iy) {
        for (int sx = x, ix = 0; sx < max_x; ++sx, ++ix) {
            // TODO: Checks done by set pixel can be moved outside the loop.
            u32 p = image.pixels[ix + iy * image.width];
            if (get_alpha(p) != 0) {
                set_pixel(sx, sy, p);
            }
        }
    }
}

// An animated_image is made up of several frames of animation,
// packed on top of each other.
typedef struct {
    u32 * pixels;
    int width;
    int height;
    int frame_count;
    int frame_duration_ms;
    int start_time_ms;
} Animated_Image;

void draw_animated_image(Animated_Image animated_image, int x, int y) {
    int time_passed = SDL_GetTicks() - animated_image.start_time_ms;
    int frames_passed = time_passed / animated_image.frame_duration_ms;
    int current_frame = frames_passed % animated_image.frame_count;
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
    Image frame = {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}

void draw_animated_image_frame(Animated_Image animated_image, int animation_frame, int x, int y) {
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * animation_frame;
    Image frame = {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}

void draw_animated_image_frames(Animated_Image animated_image, int start_frame, int end_frame, int x, int y) {
    int time_passed = SDL_GetTicks() - animated_image.start_time_ms;
    int frames_passed = time_passed / animated_image.frame_duration_ms;
    int frame_count = (end_frame - start_frame) + 1;
    int current_frame = start_frame + (frames_passed % frame_count);
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
    Image frame = {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}

void draw_animated_image_frames_and_wait(Animated_Image animated_image, int start_frame, int end_frame, int x, int y) {
    int time_passed = SDL_GetTicks() - animated_image.start_time_ms;
    int frames_passed = time_passed / animated_image.frame_duration_ms;
    int frame_count = (end_frame - start_frame) + 1;
    int current_frame = start_frame + (frames_passed % frame_count);
    if (start_frame + frames_passed > end_frame) {
        current_frame = end_frame;
    }
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
    Image frame = {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}
