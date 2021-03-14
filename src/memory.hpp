#ifndef MEMORY_HPP
#define MEMORY_HPP

#if USE_MEMORYPOOL_ITEM_DEBUG
constexpr uint32 N_MAX_MEMORYPOOL_ITEMS = 1024;
#endif

class State;

struct MemoryPool {
  uint8 *memory;
  size_t size;
  size_t used;
  uint32 n_items;
#if USE_MEMORYPOOL_ITEM_DEBUG
  const char *item_debug_names[N_MAX_MEMORYPOOL_ITEMS];
  size_t item_debug_sizes[N_MAX_MEMORYPOOL_ITEMS];
#endif
};

namespace Memory {
  void reset_memory_pool(MemoryPool *pool);
  void zero_out_memory_pool(MemoryPool *pool);
  void* push(
    MemoryPool *pool,
    size_t item_size,
    const char *item_debug_name
  );
  void print_memory_pool(MemoryPool *pool);
  void destroy_memory_pool(MemoryPool *memory_pool);
};

#endif
