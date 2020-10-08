PersistentPbo::PersistentPbo(
  uint16 texture_count, int32 width, int32 height, int32 n_components
) {
  this->texture_count = texture_count;
  this->width = width;
  this->height = height;
  this->n_components = n_components;
  this->texture_size = width * height * n_components;
  this->total_size = this->texture_size * this->texture_count;
}


void PersistentPbo::allocate_pbo() {
  glGenBuffers(1, &this->pbo);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pbo);
  GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  glBufferStorage(GL_PIXEL_UNPACK_BUFFER, this->total_size, 0, flags);
  this->memory = glMapBufferRange(
    GL_PIXEL_UNPACK_BUFFER, 0, this->total_size, flags
  );
}

void PersistentPbo::delete_pbo() {
  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
  glDeleteBuffers(1, &this->pbo);
}


uint16 PersistentPbo::get_new_idx() {
  uint16 current_idx = this->next_idx;
  this->next_idx++;
  if (this->next_idx >= this->texture_count) {
    this->next_idx = 0;
  }
  return current_idx;
}


void* PersistentPbo::get_offset_for_idx(uint16 idx) {
  return (void*)((uint64)idx * this->texture_size);
}


void* PersistentPbo::get_memory_for_idx(uint16 idx) {
  return (char*)this->memory + ((uint64)idx * this->texture_size);
}
