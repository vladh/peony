#ifndef MEMORY_HPP
#define MEMORY_HPP

class State;

class Memory {
public:
  State *state_memory;
  size_t state_memory_size;
  MemoryPool asset_memory_pool;
  MemoryPool entity_memory_pool;
  MemoryPool temp_memory_pool;

  Memory();
  ~Memory();
};

#endif
