TextureNamePool::TextureNamePool(
  Memory *memory,
  uint32 n_textures,
  uint32 texture_width, uint32 texture_height, uint32 texture_depth,
  uint32 mipmap_max_level
) {
  this->n_textures = n_textures;
  this->texture_width = texture_width;
  this->texture_height = texture_height;
  this->texture_depth = texture_depth;
  this->mipmap_max_level = mipmap_max_level;

  this->texture_names = (uint32*)memory->asset_memory_pool.push(
    sizeof(uint32) * this->n_textures, "texture_names"
  );
}

void TextureNamePool::allocate_texture_names() {
  START_TIMER(allocate_textures);

  glGenTextures(this->n_textures, this->texture_names);

  for (uint32 idx = 0; idx < this->n_textures; idx++) {
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->texture_names[idx]);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, this->mipmap_max_level);
    glTexStorage3D(
      GL_TEXTURE_2D_ARRAY, this->mipmap_max_level + 1, GL_RGBA8,
      this->texture_width, this->texture_height, this->texture_depth
    );
  }

  END_TIMER(allocate_textures);
}


uint32 TextureNamePool::get_next() {
  assert(this->idx_next < this->n_textures);
  uint32 texture_name = this->texture_names[this->idx_next];
  this->idx_next++;
  return texture_name;
}
