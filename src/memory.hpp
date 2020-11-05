#ifndef MEMORY_H
#define MEMORY_H

constexpr uint32 N_MAX_MEMORYPOOL_ITEMS = 1024;

class State;

class MemoryPool {
public:
  const char *name;
  uint8 *memory;
  size_t size;
  size_t used;
  uint32 n_items;
  const char **item_debug_names;
  size_t *item_debug_sizes;

  MemoryPool(const char *name, size_t size);
  void reset();
  void zero_out();
  void* push(size_t size, const char *item_debug_name);
  void print();

private:
  std::mutex mutex;
};

class Memory {
public:
  State *state_memory;
  size_t state_memory_size;

  MemoryPool asset_memory_pool;
  MemoryPool entity_memory_pool;
  MemoryPool temp_memory_pool;

  Memory();
};

#endif
