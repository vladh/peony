// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "util.hpp"
#include "constants.hpp"
#include "memory.hpp"
#include "intrinsics.hpp"


void *
memory::push(
    Pool *pool,
    size_t item_size,
    const char *item_debug_name
) {
    // If we had just init'd an empty pool, let's just give it some size.
    if (pool->size == 0) {
        pool->size = util::mb_to_b(256);
    }

    // If we haven't allocated anything in the pool, let's allocate something now.
    if (pool->memory == nullptr) {
        if (SETTINGS.memory_debug_logs_on) {
            logs::info("Allocating memory pool: %.2fMB (%dB)",
                util::b_to_mb((f64)pool->size), pool->size);
        }

        pool->memory = (u8*)calloc(1, pool->size);
        if (!pool->memory) {
            logs::fatal("Could not allocate memory. Buy more RAM!");
            assert(false); // A little hint for the compiler
        }

        assert(pool->memory);
    }
    assert(pool->used + item_size <= pool->size);

#if USE_MEMORYPOOL_ITEM_DEBUG
    assert(pool->n_items < MAX_N_MEMORYPOOL_ITEMS);
    pool->item_debug_names[pool->n_items] = item_debug_name;
    pool->item_debug_sizes[pool->n_items] = item_size;
#endif

    void *new_memory = pool->memory + pool->used;
    pool->used += item_size;
    pool->n_items++;

    if (SETTINGS.memory_debug_logs_on) {
        logs::info("Pusing to memory pool: %.2fMB (%dB) for %s, now at %.2fMB (%dB)",
            util::b_to_mb((f64)item_size),
            item_size, item_debug_name,
            util::b_to_mb((f64)pool->used),
            pool->used);
    }

    return new_memory;
}


void
memory::print_memory_pool(Pool *pool)
{
    logs::info("memory::Pool:");
    logs::info("  Used: %.2fMB (%dB)", util::b_to_mb((u32)pool->used), pool->used);
    logs::info("  Size: %.2fMB (%dB)", util::b_to_mb((u32)pool->size), pool->size);
    logs::info("  Items:");
    if (pool->n_items == 0) {
        logs::info("    (none)");
    }
    #if USE_MEMORYPOOL_ITEM_DEBUG
    for (u32 idx = 0; idx < pool->n_items; idx++) {
        logs::info("    %02d. %s, %.2fMB (%dB)",
            idx,
            pool->item_debug_names[idx],
            util::b_to_mb((f64)pool->item_debug_sizes[idx]),
            pool->item_debug_sizes[idx]);
    }
    #endif
}


void
memory::destroy_memory_pool(Pool *memory_pool)
{
    if (SETTINGS.memory_debug_logs_on) {
        logs::info("destroy_memory_pool");
    }
    reset_memory_pool(memory_pool);
    free(memory_pool->memory);
}


void
memory::reset_memory_pool(Pool *pool)
{
    if (SETTINGS.memory_debug_logs_on) {
        logs::info("Resetting memory pool");
    }
    pool->used = 0;
    pool->n_items = 0;
}


void
memory::zero_out_memory_pool(Pool *pool)
{
    memset(pool->memory, 0, pool->size);
    pool->used = 0;
    pool->n_items = 0;
}
