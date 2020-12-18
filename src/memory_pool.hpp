#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

#if USE_MEMORYPOOL_ITEM_DEBUG
constexpr uint32 N_MAX_MEMORYPOOL_ITEMS = 8192;
#endif

class MemoryPool {
public:
  std::mutex mutex;
  const char *name;
  uint8 *memory;
  size_t size;
  size_t used;
  uint32 n_items;
#if USE_MEMORYPOOL_ITEM_DEBUG
  const char *item_debug_names[N_MAX_MEMORYPOOL_ITEMS];
  size_t item_debug_sizes[N_MAX_MEMORYPOOL_ITEMS];
#endif

  void reset();
  void zero_out();
  void* push(size_t size, const char *item_debug_name);
  void print();
  MemoryPool(const char *name, size_t size);
  ~MemoryPool();
};

#endif
