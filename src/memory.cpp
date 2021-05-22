#include "intrinsics.hpp"
#include "logs.hpp"
#include "util.hpp"
#include "memory.hpp"


namespace memory {
  pny_internal void reset_memory_pool(MemoryPool *pool) {
#if USE_MEMORY_DEBUG_LOGS
    logs::info("Resetting memory pool");
#endif
    pool->used = 0;
    pool->n_items = 0;
  }


  pny_internal void zero_out_memory_pool(MemoryPool *pool) {
    memset(pool->memory, 0, pool->size);
    pool->used = 0;
    pool->n_items = 0;
  }
}


void* memory::push(
  MemoryPool *pool,
  size_t item_size,
  const char *item_debug_name
) {
  // If we had just init'd an empty pool, let's just give it some size.
  if (pool->size == 0) {
    pool->size = util::mb_to_b(256);
  }

  // If we haven't allocated anything in the pool, let's allocate something now.
  if (pool->memory == nullptr) {
#if USE_MEMORY_DEBUG_LOGS
    logs::info(
      "Allocating memory pool: %.2fMB (%dB)",
      util::b_to_mb((real64)pool->size),
      pool->size
    );
#endif

    pool->memory = (uint8*)malloc(pool->size);
    if (!pool->memory) {
      logs::fatal("Could not allocate memory. Buy more RAM!");
      assert(false); // A little hint for the compiler
    }

    memset(pool->memory, 0, pool->size);
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

#if USE_MEMORY_DEBUG_LOGS
  logs::info(
    "Pusing to memory pool: %.2fMB (%dB) for %s, now at %.2fMB (%dB)",
    util::b_to_mb((real64)item_size),
    item_size, item_debug_name,
    util::b_to_mb((real64)pool->used),
    pool->used
  );
#endif

  return new_memory;
}


void memory::print_memory_pool(MemoryPool *pool) {
  logs::info("MemoryPool:");
  logs::info("  Used: %.2fMB (%dB)", util::b_to_mb((uint32)pool->used), pool->used);
  logs::info("  Size: %.2fMB (%dB)", util::b_to_mb((uint32)pool->size), pool->size);
  logs::info("  Items:");
  if (pool->n_items == 0) {
    logs::info("    (none)");
  }
#if USE_MEMORYPOOL_ITEM_DEBUG
  for (uint32 idx = 0; idx < pool->n_items; idx++) {
    logs::info(
      "    %02d. %s, %.2fMB (%dB)",
      idx,
      pool->item_debug_names[idx],
      util::b_to_mb((real64)pool->item_debug_sizes[idx]),
      pool->item_debug_sizes[idx]
    );
  }
#endif
}


void memory::destroy_memory_pool(MemoryPool *memory_pool) {
#if USE_MEMORY_DEBUG_LOGS
  logs::info("destroy_memory_pool");
#endif
  reset_memory_pool(memory_pool);
  free(memory_pool->memory);
}
