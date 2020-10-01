#ifndef MEMORY_H
#define MEMORY_H

class State;

class MemoryPool {
public:
  const char *name;
  uint8 *memory;
  uint32 size;
  uint32 used;

  MemoryPool(const char *name, uint32 size);
  void reset();
  void zero_out();
  void* push(uint32 size);
  void* push(uint32 size, const char *name);
};

class Memory {
public:
  State *state_memory;
  uint32 state_memory_size;

  MemoryPool asset_memory_pool;
  MemoryPool entity_memory_pool;
  MemoryPool temp_memory_pool;

  Memory();
};

#endif
