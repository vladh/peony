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


void TextureSetAsset::generate_textures_from_pbo(
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool
) {
  this->mutex.lock();

  if (this->is_static) {
    return;
  }
  if (this->have_textures_been_generated) {
    log_warning("Tried to generate textures but they've already been generated.");
  }

  this->material_texture = texture_name_pool->get_next();
  glBindTexture(GL_TEXTURE_2D_ARRAY, this->material_texture);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, persistent_pbo->pbo);

  GLenum format;

  // TODO: When one of the textures is not set, the shader should take the "static"
  // value — if we see a texture is not set here, we should set the corresponding
  // static value to something that makes sense as a default value. Otherwise,
  // omitting textures will sometimes have weird results.

  // TODO: Textures of different sizes do not currently work. We maybe shouldn't
  // use texture arrays?

  // TODO: We should check this in a better way, maybe set a bool instead of
  // doing `strcmp()` all the time.

  if (strcmp(this->albedo_texture_path, "") != 0) {
    format = Util::get_texture_format_from_n_components(this->albedo_data_n_components);
    glTexSubImage3D(
      GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
      this->albedo_data_width, this->albedo_data_height,
      1, format, GL_UNSIGNED_BYTE,
      persistent_pbo->get_offset_for_idx(this->albedo_pbo_idx)
    );
  }

  if (strcmp(this->metallic_texture_path, "") != 0) {
    format = Util::get_texture_format_from_n_components(this->metallic_data_n_components);
    glTexSubImage3D(
      GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1,
      this->metallic_data_width, this->metallic_data_height,
      1, format, GL_UNSIGNED_BYTE,
      persistent_pbo->get_offset_for_idx(this->metallic_pbo_idx)
    );
  }

  if (strcmp(this->roughness_texture_path, "") != 0) {
    format = Util::get_texture_format_from_n_components(this->roughness_data_n_components);
    glTexSubImage3D(
      GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2,
      this->roughness_data_width, this->roughness_data_height,
      1, format, GL_UNSIGNED_BYTE,
      persistent_pbo->get_offset_for_idx(this->roughness_pbo_idx)
    );
  }

  if (strcmp(this->ao_texture_path, "") != 0) {
    format = Util::get_texture_format_from_n_components(this->ao_data_n_components);
    glTexSubImage3D(
      GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3,
      this->ao_data_width, this->ao_data_height,
      1, format, GL_UNSIGNED_BYTE,
      persistent_pbo->get_offset_for_idx(this->ao_pbo_idx)
    );
  }

  if (strcmp(this->normal_texture_path, "") != 0) {
    format = Util::get_texture_format_from_n_components(this->normal_data_n_components);
    glTexSubImage3D(
      GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4,
      this->normal_data_width, this->normal_data_height,
      1, format, GL_UNSIGNED_BYTE,
      persistent_pbo->get_offset_for_idx(this->normal_pbo_idx)
    );
  }

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  this->have_textures_been_generated = true;

  this->mutex.unlock();
}


void TextureSetAsset::copy_textures_to_pbo(PersistentPbo *persistent_pbo) {
  this->mutex.lock();

  if (this->is_static) {
    return;
  }

  if (this->have_textures_been_copied_to_pbo) {
    log_warning("Trying to load already loaded TextureSetAsset.");
    return;
  }

  uint32 image_size;

  if (strcmp(this->albedo_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->albedo_texture_path, &this->albedo_data_width,
      &this->albedo_data_height, &this->albedo_data_n_components, true
    );
    this->albedo_pbo_idx = persistent_pbo->get_new_idx();
    image_size = this->albedo_data_width *
      this->albedo_data_height *
      this->albedo_data_n_components;
    memcpy(
      persistent_pbo->get_memory_for_idx(this->albedo_pbo_idx),
      image_data,
      image_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->metallic_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->metallic_texture_path, &this->metallic_data_width,
      &this->metallic_data_height, &this->metallic_data_n_components, true
    );
    this->metallic_pbo_idx = persistent_pbo->get_new_idx();
    image_size = this->metallic_data_width *
      this->metallic_data_height *
      this->metallic_data_n_components;
    memcpy(
      persistent_pbo->get_memory_for_idx(this->metallic_pbo_idx),
      image_data,
      image_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->roughness_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->roughness_texture_path, &this->roughness_data_width,
      &this->roughness_data_height, &this->roughness_data_n_components, true
    );
    this->roughness_pbo_idx = persistent_pbo->get_new_idx();
    image_size = this->roughness_data_width *
      this->roughness_data_height *
      this->roughness_data_n_components;
    memcpy(
      persistent_pbo->get_memory_for_idx(this->roughness_pbo_idx),
      image_data,
      image_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->ao_texture_path, "") != 0) {
    unsigned char *image_data = ResourceManager::load_image(
      this->ao_texture_path, &this->ao_data_width,
      &this->ao_data_height, &this->ao_data_n_components, true
    );
    this->ao_pbo_idx = persistent_pbo->get_new_idx();
    image_size = this->ao_data_width *
      this->ao_data_height *
      this->ao_data_n_components;
    memcpy(
      persistent_pbo->get_memory_for_idx(this->ao_pbo_idx),
      image_data,
      image_size
    );
    ResourceManager::free_image(image_data);
  }

  if (strcmp(this->normal_texture_path, "") != 0) {
    this->should_use_normal_map = true;

    unsigned char* image_data = ResourceManager::load_image(
      this->normal_texture_path, &this->normal_data_width,
      &this->normal_data_height, &this->normal_data_n_components, true
    );
    this->normal_pbo_idx = persistent_pbo->get_new_idx();
    image_size = this->normal_data_width *
      this->normal_data_height *
      this->normal_data_n_components;
    memcpy(
      persistent_pbo->get_memory_for_idx(this->normal_pbo_idx),
      image_data,
      image_size
    );
    ResourceManager::free_image(image_data);
  }

  this->have_textures_been_copied_to_pbo = true;

  this->mutex.unlock();
}
