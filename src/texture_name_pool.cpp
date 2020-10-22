TextureNamePool::TextureNamePool(
  Memory *memory, uint32 n_textures, uint32 mipmap_max_level
) {
  this->n_textures = n_textures;
  this->mipmap_max_level = mipmap_max_level;

  this->n_sizes = 2;
  this->sizes[0] = 512;
  this->sizes[1] = 2048;

  this->texture_names = (uint32*)memory->asset_memory_pool.push(
    sizeof(uint32) * this->n_textures * this->n_sizes, "texture_names"
  );
}

void TextureNamePool::allocate_texture_names() {
  START_TIMER(allocate_textures);

  glGenTextures(this->n_textures * this->n_sizes, this->texture_names);

  for (uint32 idx_size = 0; idx_size < this->n_sizes; idx_size++) {
    for (uint32 idx_for_size = 0; idx_for_size < this->n_textures; idx_for_size++) {
      uint32 idx_name = (idx_size * this->n_textures) + idx_for_size;
      glBindTexture(GL_TEXTURE_2D, this->texture_names[idx_name]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, this->mipmap_max_level);
      glTexStorage2D(
        GL_TEXTURE_2D, this->mipmap_max_level + 1, GL_RGBA8,
        this->sizes[idx_size], this->sizes[idx_size]
      );
    }
  }

  END_TIMER(allocate_textures);
}


uint32 TextureNamePool::get_next(uint32 target_size) {
  for (uint32 idx_size = 0; idx_size < this->n_sizes; idx_size++) {
    if (this->sizes[idx_size] == target_size) {
      assert(this->idx_next[idx_size] < this->n_textures);
      uint32 idx_name = (idx_size * this->n_textures) + this->idx_next[idx_size];
      uint32 texture_name = this->texture_names[idx_name];
      this->idx_next[idx_size]++;
      return texture_name;
    }
  }
  log_fatal(
    "Could not make texture of size %d as there is no pool for that size.",
    target_size
  );
  return 0;
}
