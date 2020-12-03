void MemoryPool::reset() {
  this->mutex.lock();
#if USE_MEMORY_DEBUG_LOGS
  log_info("Resetting memory pool \"%s\"", this->name);
#endif
  this->used = 0;
  this->n_items = 0;
  this->mutex.unlock();
}


void MemoryPool::zero_out() {
  this->mutex.lock();
  memset(this->memory, 0, this->size);
  this->used = 0;
  this->n_items = 0;
  this->mutex.unlock();
}


void* MemoryPool::push(size_t item_size, const char *item_debug_name) {
  this->mutex.lock();
  assert(this->used + item_size <= this->size);

#if USE_MEMORYPOOL_ITEM_DEBUG
  assert(this->n_items < N_MAX_MEMORYPOOL_ITEMS);
  this->item_debug_names[this->n_items] = item_debug_name;
  this->item_debug_sizes[this->n_items] = item_size;
#endif

  void *new_memory = this->memory + this->used;
  this->used += item_size;
  this->n_items++;

#if USE_MEMORY_DEBUG_LOGS
  log_info(
    "Pusing to memory pool \"%s\": %.2fMB (%dB) for %s, now at %.2fMB (%dB)",
    this->name, B_TO_MB((real64)item_size), item_size, item_debug_name,
    B_TO_MB((real64)this->used), this->used
  );
#endif

  this->mutex.unlock();
  return new_memory;
}


void MemoryPool::print() {
  this->mutex.lock();
  log_info("MemoryPool: %s", this->name);
  log_info("  Used: %.2fMB (%dB)", B_TO_MB((real64)this->used), this->used);
  log_info("  Size: %.2fMB (%dB)", B_TO_MB((real64)this->size), this->size);
  log_info("  Items:");
  if (this->n_items == 0) {
    log_info("    (none)");
  }
#if USE_MEMORYPOOL_ITEM_DEBUG
  for (uint32 idx = 0; idx < this->n_items; idx++) {
    log_info(
      "    %02d. %s, %.2fMB (%dB)",
      idx,
      this->item_debug_names[idx],
      B_TO_MB((real64)this->item_debug_sizes[idx]),
      this->item_debug_sizes[idx]
    );
  }
#endif
  this->mutex.unlock();
}


MemoryPool::MemoryPool(
  const char *name, size_t size
) :
  name(name),
  size(size),
  used(0),
  n_items(0)
{
#if USE_MEMORY_DEBUG_LOGS
  log_info(
    "Allocating memory pool \"%s\": %.2fMB (%dB)",
    this->name, B_TO_MB((real64)this->size), this->size
  );
#endif

  this->memory = (uint8*)malloc(this->size);
  memset(this->memory, 0, this->size);
}


MemoryPool::~MemoryPool() {
#if USE_MEMORY_DEBUG_LOGS
  log_info("%s ~MemoryPool()", this->name);
#endif
  free(this->memory);
}
