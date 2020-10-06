TextureSetAsset::TextureSetAsset(
  const char *albedo_texture_path,
  const char *metallic_texture_path,
  const char *roughness_texture_path,
  const char *ao_texture_path,
  const char *normal_texture_path
) {
  this->is_static = false;
  this->albedo_texture_path = albedo_texture_path;
  this->metallic_texture_path = metallic_texture_path;
  this->roughness_texture_path = roughness_texture_path;
  this->ao_texture_path = ao_texture_path;
  this->normal_texture_path = normal_texture_path;
}


TextureSetAsset::TextureSetAsset(
  glm::vec4 albedo_static,
  real32 metallic_static,
  real32 roughness_static,
  real32 ao_static
) {
  this->is_static = true;
  this->have_textures_been_generated = true;
  this->albedo_static = albedo_static;
  this->metallic_static = metallic_static;
  this->roughness_static = roughness_static;
  this->ao_static = ao_static;
}


void TextureSetAsset::generate_textures_from_pbo(PersistentPbo *persistent_pbo) {
  if (this->is_static) {
    return;
  }
  if (this->have_textures_been_generated) {
    log_warning("Tried to generate textures but they've already been generated.");
  }

#if 0
  glGenTextures(1, &this->material_texture);
#endif
#if 1
  this->material_texture = global_texture_pool[global_texture_pool_next_idx++];
#endif
  glBindTexture(GL_TEXTURE_2D_ARRAY, this->material_texture);

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, persistent_pbo->pbo);

  // NOTE: These are pre-allocated on startup.
  glTexSubImage3D(
    GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
    this->albedo_data_width, this->albedo_data_height,
    1, GL_BGRA, GL_UNSIGNED_BYTE,
    persistent_pbo->get_offset_for_idx(this->albedo_pbo_idx)
  );
  glTexSubImage3D(
    GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1,
    this->metallic_data_width, this->metallic_data_height,
    1, GL_BGRA, GL_UNSIGNED_BYTE,
    persistent_pbo->get_offset_for_idx(this->metallic_pbo_idx)
  );
  glTexSubImage3D(
    GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2,
    this->roughness_data_width, this->roughness_data_height,
    1, GL_BGRA, GL_UNSIGNED_BYTE,
    persistent_pbo->get_offset_for_idx(this->roughness_pbo_idx)
  );
  glTexSubImage3D(
    GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3,
    this->ao_data_width, this->ao_data_height,
    1, GL_BGRA, GL_UNSIGNED_BYTE,
    persistent_pbo->get_offset_for_idx(this->ao_pbo_idx)
  );
  glTexSubImage3D(
    GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4,
    this->normal_data_width, this->normal_data_height,
    1, GL_BGRA, GL_UNSIGNED_BYTE,
    persistent_pbo->get_offset_for_idx(this->normal_pbo_idx)
  );

  this->should_use_normal_map = true;

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

#if 1
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
#endif

  this->have_textures_been_generated = true;
}


void TextureSetAsset::copy_textures_to_pbo(PersistentPbo *persistent_pbo) {
  if (this->is_static) {
    return;
  }
  if (this->have_textures_been_copied_to_pbo) {
    log_warning("Trying to load already loaded TextureSetAsset.");
    return;
  }

  this->mutex.lock();

  if (strcmp(this->albedo_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->albedo_texture_path, &this->albedo_data_width,
      &this->albedo_data_height, &this->albedo_data_n_components, true
    );
    this->albedo_pbo_idx = persistent_pbo->get_new_idx();
    memcpy(
      persistent_pbo->get_memory_for_idx(this->albedo_pbo_idx),
      image_data,
      persistent_pbo->texture_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->metallic_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->metallic_texture_path, &this->metallic_data_width,
      &this->metallic_data_height, &this->metallic_data_n_components, true
    );
    this->metallic_pbo_idx = persistent_pbo->get_new_idx();
    memcpy(
      persistent_pbo->get_memory_for_idx(this->metallic_pbo_idx),
      image_data,
      persistent_pbo->texture_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->roughness_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->roughness_texture_path, &this->roughness_data_width,
      &this->roughness_data_height, &this->roughness_data_n_components, true
    );
    this->roughness_pbo_idx = persistent_pbo->get_new_idx();
    memcpy(
      persistent_pbo->get_memory_for_idx(this->roughness_pbo_idx),
      image_data,
      persistent_pbo->texture_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->ao_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->ao_texture_path, &this->ao_data_width,
      &this->ao_data_height, &this->ao_data_n_components, true
    );
    this->ao_pbo_idx = persistent_pbo->get_new_idx();
    memcpy(
      persistent_pbo->get_memory_for_idx(this->ao_pbo_idx),
      image_data,
      persistent_pbo->texture_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->normal_texture_path, "") != 0) {
    unsigned char* image_data = ResourceManager::load_image(
      this->normal_texture_path, &this->normal_data_width,
      &this->normal_data_height, &this->normal_data_n_components, true
    );
    this->normal_pbo_idx = persistent_pbo->get_new_idx();
    memcpy(
      persistent_pbo->get_memory_for_idx(this->normal_pbo_idx),
      image_data,
      persistent_pbo->texture_size
    );
    ResourceManager::free_image(image_data);
  }

  this->mutex.unlock();

  this->have_textures_been_copied_to_pbo = true;
}
