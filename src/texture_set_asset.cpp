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
  this->is_loading_done = true;
  this->albedo_static = albedo_static;
  this->metallic_static = metallic_static;
  this->roughness_static = roughness_static;
  this->ao_static = ao_static;
}


void* TextureSetAsset::create_pbo_and_get_pointer(
  uint32 *pbo, int32 width, int32 height, int32 n_components
) {
  uint32 image_size = width * height * n_components;
  glGenBuffers(1, pbo);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, *pbo);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, 0, GL_STREAM_DRAW);
  return glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
}


void TextureSetAsset::create_pbos() {
  this->albedo_pbo_memory = create_pbo_and_get_pointer(
    &this->albedo_pbo,
    this->albedo_data_width, this->albedo_data_height,
    this->albedo_data_n_components
  );
  this->metallic_pbo_memory = create_pbo_and_get_pointer(
    &this->metallic_pbo,
    this->metallic_data_width, this->metallic_data_height,
    this->metallic_data_n_components
  );
  this->roughness_pbo_memory = create_pbo_and_get_pointer(
    &this->roughness_pbo,
    this->roughness_data_width, this->roughness_data_height,
    this->roughness_data_n_components
  );
  this->ao_pbo_memory = create_pbo_and_get_pointer(
    &this->ao_pbo,
    this->ao_data_width, this->ao_data_height,
    this->ao_data_n_components
  );
  this->normal_pbo_memory = create_pbo_and_get_pointer(
    &this->normal_pbo,
    this->normal_data_width, this->normal_data_height,
    this->normal_data_n_components
  );
}


void TextureSetAsset::copy_texture_to_gpu(
  unsigned char *data, void *pbo_memory,
  int32 width, int32 height, int32 n_components
) {
  memcpy(pbo_memory, data, width * height * n_components);
  log_info("memcpy is done");
  // NOTE: This is the image data that comes from stb_image.h.
  // After copying it to the GPU, we don't need it anymore, so we can
  // delete it here.
  ResourceManager::free_image(data);
}


void TextureSetAsset::copy_textures_to_gpu() {
  if (this->is_static) {
    return;
  }

  if (this->albedo_data) {
    copy_texture_to_gpu(
      this->albedo_data, this->albedo_pbo_memory,
      this->albedo_data_width, this->albedo_data_height,
      this->albedo_data_n_components
    );
  }
  if (this->metallic_data) {
    copy_texture_to_gpu(
      this->metallic_data, this->metallic_pbo_memory,
      this->metallic_data_width, this->metallic_data_height,
      this->metallic_data_n_components
    );
  }
  if (this->roughness_data) {
    copy_texture_to_gpu(
      this->roughness_data, this->roughness_pbo_memory,
      this->roughness_data_width, this->roughness_data_height,
      this->roughness_data_n_components
    );
  }
  if (this->ao_data) {
    copy_texture_to_gpu(
      this->ao_data, this->ao_pbo_memory,
      this->ao_data_width, this->ao_data_height,
      this->ao_data_n_components
    );
  }
  if (this->normal_data) {
    copy_texture_to_gpu(
      this->normal_data, this->normal_pbo_memory,
      this->normal_data_width, this->normal_data_height,
      this->normal_data_n_components
    );
  }
}


uint32 TextureSetAsset::generate_texture_from_pbo(
  uint32 *pbo, int32 width, int32 height, int32 n_components
) {
  START_TIMER(generate_texture_from_pbo);
  uint32 texture_id;
  START_TIMER(gl_gen_textures);
  glGenTextures(1, &texture_id);
  END_TIMER(gl_gen_textures);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  GLenum format = Util::get_texture_format_from_n_components(n_components);

#if 1
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, *pbo);
  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
  glTexImage2D(
    GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, 0
  );
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glDeleteBuffers(1, pbo);
#endif
  END_TIMER(generate_texture_from_pbo);

  return texture_id;
}


void TextureSetAsset::generate_textures_from_pbo() {
  if (this->is_static) {
    return;
  }
  if (this->is_loading_done) {
    log_warning("Tried to generate textures but they've already been generated.");
  }

  if (this->albedo_data) {
    this->albedo_texture = generate_texture_from_pbo(
      &this->albedo_pbo,
      this->albedo_data_width, this->albedo_data_height,
      this->albedo_data_n_components
    );
  }
  if (this->metallic_data) {
    this->metallic_texture = generate_texture_from_pbo(
      &this->metallic_pbo,
      this->metallic_data_width, this->metallic_data_height,
      this->metallic_data_n_components
    );
  }
  if (this->roughness_data) {
    this->roughness_texture = generate_texture_from_pbo(
      &this->roughness_pbo,
      this->roughness_data_width, this->roughness_data_height,
      this->roughness_data_n_components
    );
  }
  if (this->ao_data) {
    this->ao_texture = generate_texture_from_pbo(
      &this->ao_pbo,
      this->ao_data_width, this->ao_data_height,
      this->ao_data_n_components
    );
  }
  if (this->normal_data) {
    this->normal_texture = generate_texture_from_pbo(
      &this->normal_pbo,
      this->normal_data_width, this->normal_data_height,
      this->normal_data_n_components
    );
  }
  this->is_loading_done = true;
}


#if 0
void TextureSetAsset::load_textures_from_preloaded_data() {
  if (this->is_static) {
    return;
  }
  if (!this->is_image_data_preloaded) {
    log_warning("Trying to load textures, but no data preloaded.");
    return;
  }

  this->mutex.lock();
  if (this->albedo_data) {
    this->albedo_texture = ResourceManager::load_texture_from_image_data_and_free(
      this->albedo_data, &this->albedo_data_width,
      &this->albedo_data_height, &this->albedo_data_n_components
    );
  }
  if (this->metallic_data) {
    this->metallic_texture = ResourceManager::load_texture_from_image_data_and_free(
      this->metallic_data, &this->metallic_data_width,
      &this->metallic_data_height, &this->metallic_data_n_components
    );
  }
  if (this->roughness_data) {
    this->roughness_texture = ResourceManager::load_texture_from_image_data_and_free(
      this->roughness_data, &this->roughness_data_width,
      &this->roughness_data_height, &this->roughness_data_n_components
    );
  }
  if (this->ao_data) {
    this->ao_texture = ResourceManager::load_texture_from_image_data_and_free(
      this->ao_data, &this->ao_data_width,
      &this->ao_data_height, &this->ao_data_n_components
    );
  }
  if (this->normal_data) {
    this->normal_texture = ResourceManager::load_texture_from_image_data_and_free(
      this->normal_data, &this->normal_data_width,
      &this->normal_data_height, &this->normal_data_n_components
    );
  }
  this->mutex.unlock();

  this->is_loading_done = true;
}
#endif


void TextureSetAsset::preload_image_data() {
  if (this->is_static) {
    return;
  }
  if (this->is_image_data_preloaded) {
    log_warning("Trying to load already loaded TextureSetAsset.");
    return;
  }

  this->mutex.lock();
  if (strcmp(this->albedo_texture_path, "") != 0) {
    this->albedo_data = ResourceManager::load_image(
      this->albedo_texture_path, &this->albedo_data_width,
      &this->albedo_data_height, &this->albedo_data_n_components, true
    );
  }
  if (strcmp(this->metallic_texture_path, "") != 0) {
    this->metallic_data = ResourceManager::load_image(
      this->metallic_texture_path, &this->metallic_data_width,
      &this->metallic_data_height, &this->metallic_data_n_components, true
    );
  }
  if (strcmp(this->roughness_texture_path, "") != 0) {
    this->roughness_data = ResourceManager::load_image(
      this->roughness_texture_path, &this->roughness_data_width,
      &this->roughness_data_height, &this->roughness_data_n_components, true
    );
  }
  if (strcmp(this->ao_texture_path, "") != 0) {
    this->ao_data = ResourceManager::load_image(
      this->ao_texture_path, &this->ao_data_width,
      &this->ao_data_height, &this->ao_data_n_components, true
    );
  }
  if (strcmp(this->normal_texture_path, "") != 0) {
    this->normal_data = ResourceManager::load_image(
      this->normal_texture_path, &this->normal_data_width,
      &this->normal_data_height, &this->normal_data_n_components, true
    );
  }
  this->mutex.unlock();

  this->is_image_data_preloaded = true;
}
