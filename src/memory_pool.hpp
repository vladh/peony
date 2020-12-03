#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#if USE_MEMORYPOOL_ITEM_DEBUG
constexpr uint32 N_MAX_MEMORYPOOL_ITEMS = 8192;
#endif

class MemoryPool {
public:
  const char *name;
  uint8 *memory;
  size_t size;
  size_t used;
  uint32 n_items;
#if USE_MEMORYPOOL_ITEM_DEBUG
  const char *item_debug_names[N_MAX_MEMORYPOOL_ITEMS];
  size_t item_debug_sizes[N_MAX_MEMORYPOOL_ITEMS];
#endif

  MemoryPool(const char *name, size_t size);
  ~MemoryPool();
  void reset();
  void zero_out();
  void* push(size_t size, const char *item_debug_name);
  void print();

private:
  std::mutex mutex;
};

#endif
