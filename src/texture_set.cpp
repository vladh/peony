void TextureSet::add(Texture texture) {
  this->textures.push(texture);
}


void TextureSet::set_albedo_static(glm::vec4 new_albedo_static) {
  this->albedo_static = new_albedo_static;
}


void TextureSet::set_metallic_static(real32 new_metallic_static) {
  this->metallic_static = new_metallic_static;
}


void TextureSet::set_roughness_static(real32 new_roughness_static) {
  this->roughness_static = new_roughness_static;
}


void TextureSet::set_ao_static(real32 new_ao_static) {
  this->ao_static = new_ao_static;
}


void TextureSet::copy_textures_to_pbo(PersistentPbo *persistent_pbo) {
  this->mutex.lock();

  for (uint32 idx = 0; idx < this->textures.size; idx++) {
    Texture *texture = this->textures.get(idx);
    if (texture->texture_name) {
      continue;
    }
    unsigned char *image_data = ResourceManager::load_image(
      texture->path, &texture->width, &texture->height,
      &texture->n_components, true
    );
    texture->pbo_idx_for_copy = persistent_pbo->get_new_idx();
    memcpy(
      persistent_pbo->get_memory_for_idx(texture->pbo_idx_for_copy),
      image_data,
      texture->width * texture->height * texture->n_components
    );
    ResourceManager::free_image(image_data);
  }

  this->mutex.unlock();
}


void TextureSet::generate_textures_from_pbo(
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool
) {
  this->mutex.lock();
  if (this->have_textures_been_generated) {
    log_warning("Tried to generate textures but they've already been generated.");
  }
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, persistent_pbo->pbo);

  for (uint32 idx = 0; idx < this->textures.size; idx++) {
    Texture *texture = this->textures.get(idx);
    if (texture->texture_name != 0) {
      continue;
    }
    texture->texture_name = texture_name_pool->get_next();
    glBindTexture(GL_TEXTURE_2D, texture->texture_name);
    glTexSubImage2D(
      GL_TEXTURE_2D, 0, 0, 0,
      texture->width, texture->height,
      Util::get_texture_format_from_n_components(texture->n_components),
      GL_UNSIGNED_BYTE,
      persistent_pbo->get_offset_for_idx(texture->pbo_idx_for_copy)
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    if (texture->type == TEXTURE_NORMAL) {
      this->should_use_normal_map = true;
    }
  }

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  this->have_textures_been_generated = true;
  this->mutex.unlock();
}
