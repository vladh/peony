#include "memory.hpp"
#include "log.hpp"

MemoryPool memory_make_memory_pool(const char *name, uint32 size) {
  log_info(
    "Allocating memory pool '%s': %.2fMB (%dB)",
    name, (real64)size / 1024 / 1024, size
  );
  log_newline();

  MemoryPool pool;
  pool.name = name;
  pool.size = size;
  pool.used = 0;
  pool.memory = (uint8*)malloc(pool.size);
  memset(pool.memory, 0, pool.size);
  return pool;
}

void* memory_push_memory_to_pool(MemoryPool *pool, uint32 size) {
  log_info(
    "Pusing to memory pool '%s': %.2fMB (%dB)",
    pool->name, (real64)size / 1024 / 1024, size
  );
  log_newline();

  assert(pool->used + size <= pool->size);
  void *new_memory = pool->memory + pool->used;
  pool->used += size;
  return new_memory;
}
