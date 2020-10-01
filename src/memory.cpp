MemoryPool::MemoryPool(const char *new_name, uint32 new_size) {
  log_info(
    "Allocating memory pool \"%s\": %.2fMB (%dB)",
    new_name, B_TO_MB((real64)new_size), new_size
  );

  this->name = new_name;
  this->size = new_size;
  this->used = 0;
  this->n_items = 0;
  this->memory = (uint8*)malloc(this->size);
  memset(this->memory, 0, this->size);

  // NOTE: The item_debug_names and item_debug_sizes for each MemoryPool
  // is allocated memory that we won't be "keeping track of" (e.g. printing
  // for debugging purposes) in the application. We should remember it's there.
  this->item_debug_names = (const char**)malloc(sizeof(char*) * this->size);
  this->item_debug_sizes = (uint32*)malloc(sizeof(uint32) * this->size);
}


void MemoryPool::reset() {
  this->used = 0;
  this->n_items = 0;
}


void MemoryPool::zero_out() {
  memset(this->memory, 0, this->size);
  reset();
}


#if 0
void* MemoryPool::push(uint32 pushee_size) {
  assert(this->used + pushee_size <= this->size);
  void *new_memory = this->memory + this->used;
  this->used += pushee_size;
  return new_memory;
}
#endif


void* MemoryPool::push(uint32 item_size, const char *item_debug_name) {
#if 0
  log_info(
    "Pusing to memory pool \"%s\": %.2fMB (%dB) for %s, now at %.2fMB (%dB)",
    this->name, B_TO_MB((real64)), size, debug_name,
    B_TO_MB((real64)pool->used), pool->used
  );
#endif
  assert(this->used + item_size <= this->size);
  void *new_memory = this->memory + this->used;
  this->used += item_size;
  this->item_debug_names[this->n_items] = item_debug_name;
  this->item_debug_sizes[this->n_items] = item_size;
  this->n_items++;
  return new_memory;
}


void MemoryPool::print() {
  log_info("MemoryPool: %s", this->name);
  log_info("  Used: %.2fMB (%dB)", B_TO_MB((real64)this->used), this->used);
  log_info("  Size: %.2fMB (%dB)", B_TO_MB((real64)this->size), this->size);
  log_info("  Items:");
  if (this->n_items == 0) {
    log_info("    (none)");
  }
  for (uint32 idx = 0; idx < this->n_items; idx++) {
    log_info(
      "    %02d. %s, %.2fMB (%dB)",
      idx,
      this->item_debug_names[idx],
      B_TO_MB((real64)this->item_debug_sizes[idx]),
      this->item_debug_sizes[idx]
    );
  }
}


Memory::Memory(
) : state_memory_size(sizeof(State)),
  asset_memory_pool("assets", MB_TO_B(256)),
  entity_memory_pool("entities", MB_TO_B(64)),
  temp_memory_pool("temp", MB_TO_B(256))
{
  this->state_memory = (State*)malloc(this->state_memory_size);
  memset(this->state_memory, 0, this->state_memory_size);
}
