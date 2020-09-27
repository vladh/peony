#ifndef MEMORY_H
#define MEMORY_H

struct State;

struct MemoryPool {
  const char *name;
  uint8 *memory;
  uint32 size;
  uint32 used;
};

struct Memory {
  State *state_memory;
  uint32 state_memory_size;

  MemoryPool asset_memory_pool;
  MemoryPool entity_memory_pool;
  MemoryPool temp_memory_pool;
};

MemoryPool memory_make_memory_pool(const char *name, uint32 size);
void memory_reset_pool(MemoryPool *pool);
void memory_zero_out_pool(MemoryPool *pool);
void* memory_push_memory_to_pool(MemoryPool *pool, uint32 size);
void* memory_push_memory_to_pool(MemoryPool *pool, uint32 size, const char *name);

#endif
