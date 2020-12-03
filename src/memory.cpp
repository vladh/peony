Memory::Memory(
) :
  state_memory_size(sizeof(State)),
  asset_memory_pool("assets", MB_TO_B(64)),
  entity_memory_pool("entities", MB_TO_B(64)),
  temp_memory_pool("temp", MB_TO_B(256))
{
  this->state_memory = (State*)malloc(this->state_memory_size);
  memset(this->state_memory, 0, this->state_memory_size);

#if USE_MEMORY_DEBUG_LOGS
  log_info(
    "Allocating memory for state: %.2fMB (%dB)",
    B_TO_MB((real64)this->state_memory_size), this->state_memory_size
  );
#endif
}


Memory::~Memory() {
#if USE_MEMORY_DEBUG_LOGS
  log_info("~Memory()");
#endif
  free(this->state_memory);
}
