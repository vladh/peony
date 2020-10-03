TextureSetAsset::TextureSetAsset(
  const char *albedo_texture_path,
  const char *metallic_texture_path,
  const char *roughness_texture_path,
  const char *ao_texture_path,
  const char *normal_texture_path
) {
  this->is_loaded = false;
  this->is_static = false;

  this->albedo_texture = 0;
  this->metallic_texture = 0;
  this->roughness_texture = 0;
  this->ao_texture = 0;
  this->normal_texture = 0;

  this->albedo_texture_path = albedo_texture_path;
  this->metallic_texture_path = metallic_texture_path;
  this->roughness_texture_path = roughness_texture_path;
  this->ao_texture_path = ao_texture_path;
  this->normal_texture_path = normal_texture_path;

  this->albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  this->metallic_static = -1.0f;
  this->roughness_static = -1.0f;
  this->ao_static = -1.0f;
}

TextureSetAsset::TextureSetAsset(
  glm::vec4 albedo_static,
  real32 metallic_static,
  real32 roughness_static,
  real32 ao_static
) {
  this->is_loaded = true;
  this->is_static = true;

  this->albedo_texture = 0;
  this->metallic_texture = 0;
  this->roughness_texture = 0;
  this->ao_texture = 0;
  this->normal_texture = 0;

  this->albedo_texture_path = "";
  this->metallic_texture_path = "";
  this->roughness_texture_path = "";
  this->ao_texture_path = "";
  this->normal_texture_path = "";

  this->albedo_static = albedo_static;
  this->metallic_static = metallic_static;
  this->roughness_static = roughness_static;
  this->ao_static = ao_static;
}

void TextureSetAsset::load() {
  if (this->is_static) {
    return;
  }
  if (this->is_loaded) {
    log_warning("Trying to load already loaded TextureSetAsset.");
    return;
  }

  this->mutex.lock();
  if (strcmp(this->albedo_texture_path, "") != 0) {
    this->albedo_texture = ResourceManager::load_texture_from_file(
      this->albedo_texture_path
    );
  }
  if (strcmp(this->metallic_texture_path, "") != 0) {
    this->metallic_texture = ResourceManager::load_texture_from_file(
      this->metallic_texture_path
    );
  }
  if (strcmp(this->roughness_texture_path, "") != 0) {
    this->roughness_texture = ResourceManager::load_texture_from_file(
      this->roughness_texture_path
    );
  }
  if (strcmp(this->ao_texture_path, "") != 0) {
    this->ao_texture = ResourceManager::load_texture_from_file(
      this->ao_texture_path
    );
  }
  if (strcmp(this->normal_texture_path, "") != 0) {
    this->normal_texture = ResourceManager::load_texture_from_file(
      this->normal_texture_path
    );
  }
  this->mutex.unlock();

  this->is_loaded = true;
}
