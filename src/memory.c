//
// memory.c
//
// This file contains:
//     - Memory pool allocators.
//     - Memory utility functions.
//

//
// General memory related functions.
//

static inline u64 megabytes(u64 count)
{
    return count * 1024 * 1024;
}

// All memory allocations align to the platform's maximum primitive size.
#define MAX_ALIGNMENT_BYTES (sizeof(max_align_t))

// Round byte_count off to the next multiple of the desired alignment.
static inline u64 align_byte_count(u64 byte_count)
{
    byte_count += (MAX_ALIGNMENT_BYTES - 1);
    byte_count &= ~(MAX_ALIGNMENT_BYTES - 1);
    return byte_count;
}

void set_memory(void * memory, u64 byte_count, u8 value)
{
    while (byte_count)
    {
        *( u8 *)memory++ = value;
        --byte_count;
    }
}

// Perform a copy of the data at src to dest of byte_count bytes.
void copy_memory(void * src, void * dest, u64 byte_count)
{
    if (byte_count & (~(u64)7))
    {
        // Copy 8 bytes at a time.
        u64 * src64 = src;
        u64 * dest64 = dest;
        u64 count = byte_count / 8;
        for (u64 i = 0; i < count; ++i) *dest64++ = *src64++;
    }
    else if (byte_count & (~(u64)3))
    {
        // Copy 4 bytes at a time.
        u32 * src32 = src;
        u32 * dest32 = dest;
        u64 count = byte_count / 4;
        for (u64 i = 0; i < count; ++i) *dest32++ = *src32++;
    }
    else
    {
        // Fall back to byte-by-byte copy.
        while (byte_count-- > 0) *(u8 *)dest++ = *(u8 *)src++;
    }
}

// Returns true the given memory is byte-for-byte equivalent.
bool equal(void * a, void * b, u64 byte_count)
{
    if (a == b) return true;
    while (byte_count && *(u8 *)a++ == *(u8 *)b++) --byte_count;
    return byte_count == 0;
}

//
// Memory Pools.
//
// Memory pools are simple stack-like structures that have a fixed amount of
// memory under their control. When an allocation is made from a pool, it
// selects some memory from the top of its pool and returns that pointer.
//
// All allocations are guaranteed to be aligned correctly for the largest type
// available on the machine.
//

typedef struct
{
    u8 * memory;
    u64 bytes_available;
    u64 bytes_filled;
    u64 byte_count_of_last_alloc;
} Memory_Pool;

#define PERSIST_POOL 0
#define SCENE_POOL 1
#define FRAME_POOL 2

Memory_Pool memory_pools[] = {
    [PERSIST_POOL] = { NULL, 0, 0, 0 },
    [SCENE_POOL]   = { NULL, 0, 0, 0 },
    [FRAME_POOL]   = { NULL, 0, 0, 0 },
};

// Allocates a number of bytes from the given memory pool.
// Returns NULL if the allocation was unsuccessful.
void * pool_alloc(int pool_index, u64 byte_count)
{
    void * result = NULL;
    byte_count = align_byte_count(byte_count);
    Memory_Pool * pool = &memory_pools[pool_index];
    if (pool->bytes_filled + byte_count <= pool->bytes_available)
    {
        result = pool->memory + pool->bytes_filled;
        pool->bytes_filled += byte_count;
        pool->byte_count_of_last_alloc = byte_count;
    }
    return result;
}

// Deallocate the last item that was allocated.
// Useful for very temporary usage of memory.
void pool_unalloc(int pool_index)
{
    Memory_Pool * pool = &memory_pools[pool_index];
    pool->bytes_filled -= pool->byte_count_of_last_alloc;
    pool->byte_count_of_last_alloc = 0;
}

void flush_pool(int pool_index)
{
    // TODO: Should flushed memory be zeroed?
    memory_pools[pool_index].bytes_filled = 0;
    memory_pools[pool_index].byte_count_of_last_alloc = 0;
}

// Returns true if the pointer points to memory inside the given pool.
bool was_allocated_by_pool(int pool_index, void * pointer)
{
    u64 p = (u64)pointer;
    u64 low = (u64)memory_pools[pool_index].memory;
    u64 high = (u64)((u8 *)low + memory_pools[pool_index].bytes_available);
    return p >= low && p < high;
}

// Create an allocated copy of any memory.
void * clone_memory(int pool_index, void * src, u64 byte_count)
{
    void * memory = pool_alloc(pool_index, byte_count);
    if (memory) copy_memory(src, memory, byte_count);
    return memory;
}

// Allocates the memory pools.
// Returns false if the allocation did not succeed.
// persist_byte_count is ignored if POOL_STATIC_ALLOCATE is defined.
bool init_memory_pools(u64 persist_byte_count, u64 scene_byte_count, u64 frame_byte_count)
{
#ifdef POOL_STATIC_ALLOCATE
    persist_byte_count = POOL_STATIC_PERSIST_BYTE_COUNT;
#endif

    // Scene and frame pools must fit inside the persistent pool.
    if (scene_byte_count + frame_byte_count > persist_byte_count) return false;

#ifdef POOL_STATIC_ALLOCATE
    // If the storage is not too large, it is totally safe to statically allocate
    // this memory. 2GB is a reasonable maximum to assume.
    // https://software.intel.com/en-us/articles/memory-limits-applications-windows
    static u8 static_memory[POOL_STATIC_PERSIST_BYTE_COUNT];
    void * memory = static_memory;
#else
    // Allocate the memory at run time.
    // mmap is preferred to malloc as it will not maintain internal storage,
    // all memory allocated by mmap will be under our control.
    void * memory = mmap(0,
        persist_byte_count,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        0, 0);
#endif

    if (!memory) return false;

    // The persistent pool handles this allocated memory.
    memory_pools[PERSIST_POOL].memory = memory;
    memory_pools[PERSIST_POOL].bytes_available = persist_byte_count;
    memory_pools[PERSIST_POOL].bytes_filled = 0;

    // The scene and frame pools are allocated from the persistent pool.
    memory_pools[SCENE_POOL].memory = pool_alloc(PERSIST_POOL, scene_byte_count);
    memory_pools[SCENE_POOL].bytes_available = scene_byte_count;
    memory_pools[SCENE_POOL].bytes_filled = 0;

    memory_pools[FRAME_POOL].memory = pool_alloc(PERSIST_POOL, frame_byte_count);
    memory_pools[FRAME_POOL].bytes_available = frame_byte_count;
    memory_pools[FRAME_POOL].bytes_filled = 0;

    return true;
}

//
// DEBUG:
//

void print_memory_stats()
{
    printf("Memory Pool Stats:\n");

    printf("Persist: %9llu / %9llu (%02.0f%%), %9llu\n",
        memory_pools[PERSIST_POOL].bytes_filled,
        memory_pools[PERSIST_POOL].bytes_available,
        memory_pools[PERSIST_POOL].bytes_filled / (f32)memory_pools[PERSIST_POOL].bytes_available * 100,
        memory_pools[PERSIST_POOL].byte_count_of_last_alloc);

    printf("Scene:   %9llu / %9llu (%02.0f%%), %9llu\n",
        memory_pools[SCENE_POOL].bytes_filled,
        memory_pools[SCENE_POOL].bytes_available,
        memory_pools[SCENE_POOL].bytes_filled / (f32)memory_pools[SCENE_POOL].bytes_available * 100,
        memory_pools[SCENE_POOL].byte_count_of_last_alloc);

    printf("Frame:   %9llu / %9llu (%02.0f%%), %9llu\n",
        memory_pools[FRAME_POOL].bytes_filled,
        memory_pools[FRAME_POOL].bytes_available,
        memory_pools[FRAME_POOL].bytes_filled / (f32)memory_pools[FRAME_POOL].bytes_available * 100,
        memory_pools[FRAME_POOL].byte_count_of_last_alloc);
}
