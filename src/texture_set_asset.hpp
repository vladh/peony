#ifndef TEXTURE_SET_ASSET_H
#define TEXTURE_SET_ASSET_H

class TextureSetAsset : public Asset {
public:
  bool32 is_static = false;
  bool32 is_image_data_preloaded = false;
  bool32 is_loading_done = false;

  // Loaded texture maps.
  uint32 albedo_texture = 0;
  uint32 metallic_texture = 0;
  uint32 roughness_texture = 0;
  uint32 ao_texture = 0;
  uint32 normal_texture = 0;

  const char *albedo_texture_path = "";
  const char *metallic_texture_path = "";
  const char *roughness_texture_path = "";
  const char *ao_texture_path = "";
  const char *normal_texture_path = "";

  unsigned char *albedo_data = nullptr;
  unsigned char *metallic_data = nullptr;
  unsigned char *roughness_data = nullptr;
  unsigned char *ao_data = nullptr;
  unsigned char *normal_data = nullptr;
  int32 albedo_data_width;
  int32 metallic_data_width;
  int32 roughness_data_width;
  int32 ao_data_width;
  int32 normal_data_width;
  int32 albedo_data_height;
  int32 metallic_data_height;
  int32 roughness_data_height;
  int32 ao_data_height;
  int32 normal_data_height;
  int32 albedo_data_n_components;
  int32 metallic_data_n_components;
  int32 roughness_data_n_components;
  int32 ao_data_n_components;
  int32 normal_data_n_components;

  // Hardcoded values for when we can't load a texture.
  glm::vec4 albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  real32 metallic_static = -1.0f;
  real32 roughness_static = -1.0f;
  real32 ao_static = -1.0f;

  std::mutex mutex;

  TextureSetAsset(
    const char *albedo_texture_path,
    const char *metallic_texture_path,
    const char *roughness_texture_path,
    const char *ao_texture_path,
    const char *normal_texture_path
  );
  TextureSetAsset(
    glm::vec4 albedo_static,
    real32 metallic_static,
    real32 roughness_static,
    real32 ao_static
  );
  void preload_image_data();
  void load_textures_from_preloaded_data();
};

#endif
