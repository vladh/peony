// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

#define MEMORY_PUSH(pool, type, debug_name) \
    (type*)memory::push(pool, sizeof(type), debug_name)

class memory {
public:
#if USE_MEMORYPOOL_ITEM_DEBUG
    constexpr uint32 MAX_N_MEMORYPOOL_ITEMS = 1024;
#endif

    struct MemoryPool {
        uint8 *memory;
        size_t size;
        size_t used;
        uint32 n_items;
        #if USE_MEMORYPOOL_ITEM_DEBUG
        const char *item_debug_names[MAX_N_MEMORYPOOL_ITEMS];
        size_t item_debug_sizes[MAX_N_MEMORYPOOL_ITEMS];
        #endif
    };

    static void * push(
        MemoryPool *pool,
        size_t item_size,
        const char *item_debug_name
    );
    static void print_memory_pool(MemoryPool *pool);
    static void destroy_memory_pool(MemoryPool *memory_pool);

private:
    static void reset_memory_pool(MemoryPool *pool);
    static void zero_out_memory_pool(MemoryPool *pool);
};
