//
// common.c
//
// This file contains:
//     - Primitive type definitions.
//     - Pseudo-random number generator and utilities.
//     - Error reporting functions.
//

//
// Primitive type definitions.
//

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef _Bool bool;
#define true 1
#define false 0

//
// Common utility functions.
//

#define swap(a, b) { __typeof__(a) tmp = a; a = b; b = tmp; }
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(low, value, high) max(low, (min(value, high)))

//
// Pseudo-random number generator.
// (Xoroshiro128+)
//

u64 random_seed[2] = { (u64)__DATE__, (u64)__TIME__ };

u64 random_u64() {
    // Get the next random number.
    u64 s0 = random_seed[0];
    u64 s1 = random_seed[1];
    u64 result = s0 + s1;

    // Increment the generator.
    s1 ^= s0;
    #define LS(x, k) ((x << k) | (x >> (64 - k)))
    random_seed[0] = LS(s0, 55) ^ s1 ^ (s1 << 14);
    random_seed[1] = LS(s1, 36);
    #undef LS

    // Return the number.
    return result;
}

// Set the seed for the pseudo-random number generator.
void set_seed(u64 a, u64 b) {
    random_seed[0] = a;
    random_seed[1] = b;
    // The first few iterations generate poor results,
    // so run the generator a few times to avoid this.
    for (int i = 0; i < 64; ++i) random_u64();
}

// Get a random f32 between 0.0 and 1.0.
f32 random_f32() {
    return (f32)random_u64() / (f32)UINT64_MAX;
}

// Get a random f32 between low and high.
f32 random_f32_range(f32 low, f32 high) {
    f32 d = fabsf(high - low);
    return random_f32() * d + low;
}

// Get a random int between low and high, inclusive.
int random_int_range(int low, int high) {
    int d = abs(high - low) + 1;
    return random_f32() * d + low;
}

// Get a random boolean.
bool chance(f32 chance_to_be_true) {
    return random_f32() <= chance_to_be_true;
}

//
// Error reporting.
//

// Print a message and exit the program.
// Also attempts to display a pop-up error message.
void panic_exit(char * message, ...) {
    va_list args;
    va_start(args, message);
    char buffer[256];
    vsnprintf(buffer, 256, message, args);
    va_end(args);
    puts(buffer);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buffer, NULL);
    exit(1);
}

// Same as above but does not exit the program.
void issue_warning(char * message, ...) {
    va_list args;
    va_start(args, message);
    char buffer[256];
    vsnprintf(buffer, 256, message, args);
    va_end(args);
    puts(buffer);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning!", buffer, NULL);
}

//
// Generic type printer. (Requires C11)
//

#define GFMT(x) _Generic((x),           \
    bool:                     "%d\n",   \
    char:                     "%c\n",   \
    signed char:              "%hhd\n", \
    unsigned char:            "%hhu\n", \
    signed short:             "%hd\n",  \
    unsigned short:           "%hu\n",  \
    signed int:               "%d\n",   \
    unsigned int:             "%u\n",   \
    long int:                 "%ld\n",  \
    unsigned long int:        "%lu\n",  \
    long long int:            "%lld\n", \
    unsigned long long int:   "%llu\n", \
    float:                    "%f\n",   \
    double:                   "%f\n",   \
    long double:              "%Lf\n",  \
    char *:                   "%s\n",   \
    signed char *:            "%p\n",   \
    unsigned char *:          "%p\n",   \
    signed short *:           "%p\n",   \
    unsigned short *:         "%p\n",   \
    signed int *:             "%p\n",   \
    unsigned int *:           "%p\n",   \
    long int *:               "%p\n",   \
    unsigned long int *:      "%p\n",   \
    long long int *:          "%p\n",   \
    unsigned long long int *: "%p\n",   \
    float *:                  "%p\n",   \
    double *:                 "%p\n",   \
    long double *:            "%p\n",   \
    void *:                   "%p\n",   \
    default:                  "")
#define put(x) printf(GFMT(x),x)
