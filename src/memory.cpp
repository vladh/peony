MemoryPool::MemoryPool(const char *new_name, uint32 new_size) {
  log_info(
    "Allocating memory pool \"%s\": %.2fMB (%dB)",
    new_name, (real64)new_size / 1024 / 1024, new_size
  );

  this->name = new_name;
  this->size = new_size;
  this->used = 0;
  this->memory = (uint8*)malloc(this->size);
  memset(this->memory, 0, this->size);
}


void MemoryPool::reset() {
  this->used = 0;
}


void MemoryPool::zero_out() {
  memset(this->memory, 0, this->size);
  reset();
}


void* MemoryPool::push(uint32 pushee_size) {
  assert(this->used + pushee_size <= this->size);
  void *new_memory = this->memory + this->used;
  this->used += pushee_size;
  return new_memory;
}


void* MemoryPool::push(uint32 pushee_size, const char *debug_name) {
#if 0
  log_info(
    "Pusing to memory pool \"%s\": %.2fMB (%dB) for %s, now at %.2fMB (%dB)",
    this->name, (real64)size / 1024 / 1024, size, debug_name,
    (real64)pool->used / 1024 / 1024, pool->used
  );
#endif
  return push(pushee_size);
}


Memory::Memory(
) : state_memory_size(sizeof(State)),
  asset_memory_pool("assets", MEGABYTES(256)),
  entity_memory_pool("entities", MEGABYTES(64)),
  temp_memory_pool("temp", MEGABYTES(256))
{
  this->state_memory = (State*)malloc(this->state_memory_size);
  memset(this->state_memory, 0, this->state_memory_size);
}
