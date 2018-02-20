//
// common.c
//
// This file contains:
//     - Primitive type definitions.
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

typedef void (*Basic_Function)(void);

#define BIT(n) (1llu << (n))

// Most significant bit for some types.
#define MSB8 BIT(7)
#define MSB16 BIT(15)
#define MSB32 BIT(31)
#define MSB64 BIT(63)

#define swap(a, b) { __typeof__(a) tmp = a; a = b; b = tmp; }

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(low, value, high) max(low, (min(val, high)))



//
// Pseudo-random number generator.
// (Xoroshiro128+)
//

u64 random_seed[2] = { (u64)__DATE__, (u64)__TIME__ };

u64 random_u64() {
    u64 s0 = random_seed[0];
    u64 s1 = random_seed[1];
    u64 result = s0 + s1;
    s1 ^= s0;
    #define LS(x, k) ((x << k) | (x >> (64 - k)))
    random_seed[0] = LS(s0, 55) ^ s1 ^ (s1 << 14);
    random_seed[1] = LS(s1, 36);
    #undef LS
    return result;
}

// Set the seed for the pseudo-random number generator.
void set_seed(u64 a, u64 b) {
    random_seed[0] = a;
    random_seed[1] = b;
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
// Common data types.
//

// Flexible array.
// Modified version of STB stretchy buffer (which is public domain).
#define flex_free(a)   ((a) ? free(flex__raw(a)),0 : 0)
#define flex_push(a,v) (flex__maybegrow(a,1), (a)[flex__raw_count(a)++] = (v))
#define flex_count(a)  ((a) ? flex__raw_count(a) : 0)
#define flex_add(a,n)  (flex__maybegrow(a,n), flex__raw_count(a)+=(n), &(a)[flex__raw_count(a)-(n)])
#define flex_last(a)   ((a)[flex__raw_count(a)-1])

#define flex__raw(a) ((int *) (a) - 2)
#define flex__raw_size(a) flex__raw(a)[0]
#define flex__raw_count(a) flex__raw(a)[1]
#define flex__needgrow(a,n) ((a) == 0 || flex__raw_count(a)+(n) >= flex__raw_size(a))
#define flex__maybegrow(a,n) (flex__needgrow(a,(n)) ? flex__grow(a,n) : 0)
#define flex__grow(a,n) (*((void **)&(a)) = flex__raw_grow((a), (n), sizeof(*(a))))

static inline void * flex__raw_grow(void * flex_array, int increment, int item_size) {
    int double_size = flex_array ? 2 * flex__raw_size(flex_array) : 0;
    int min_needed  = flex_count(flex_array) + increment;
    int size = double_size > min_needed ? double_size : min_needed;
    int * new_head = realloc(flex_array ? flex__raw(flex_array) : 0, item_size * size + sizeof(int) * 2);
    if (new_head) {
        if (!flex_array) new_head[1] = 0;
        new_head[0] = size;
        return new_head + 2;
    } else {
        return (void *) (2 * sizeof(int));
    }
}

// Bit manipulation functions for any size bit array.
// Bit order is descending (0 is highest).
bool get_bit(void * bit_array, u64 bit) {
    u8 * ba8 = bit_array;
    u64 bucket = bit / 8;
    u64 offset = bit % 8;
    return ba8[bucket] & (MSB8 >> offset);
}

void set_bit(void * bit_array, u64 bit, bool on) {
    u8 * ba8 = bit_array;
    u64 bucket = bit / 8;
    u64 offset = bit % 8;
    if (on) {
        ba8[bucket] |= (MSB8 >> offset);
    } else {
        ba8[bucket] &= ~(MSB8 >> offset);
    }
}



//
// Error reporting / debug functions.
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

// Generic type printer.
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
