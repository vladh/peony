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
