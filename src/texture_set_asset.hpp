#ifndef TEXTURE_SET_ASSET_H
#define TEXTURE_SET_ASSET_H

class TextureSetAsset : public Asset {
public:
  bool32 is_static = false;
  bool32 is_image_data_preloaded = false;
  bool32 is_loading_done = false;
  bool32 should_use_normal_map = false;

  // Loaded texture maps.
  uint32 material_texture = 0;
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
  uint32 albedo_pbo;
  uint32 metallic_pbo;
  uint32 roughness_pbo;
  uint32 ao_pbo;
  uint32 normal_pbo;
  uint16 albedo_pbo_idx;
  uint16 metallic_pbo_idx;
  uint16 roughness_pbo_idx;
  uint16 ao_pbo_idx;
  uint16 normal_pbo_idx;
  void *albedo_pbo_memory;
  void *metallic_pbo_memory;
  void *roughness_pbo_memory;
  void *ao_pbo_memory;
  void *normal_pbo_memory;

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
  void create_pbos(PersistentPbo *persistent_pbo);
  void copy_textures_to_pbo(PersistentPbo *persistent_pbo);
  void generate_textures_from_pbo(PersistentPbo *persistent_pbo);
#if 0
  void load_textures_from_preloaded_data();
#endif

private:
#if 0
  void* create_pbo_and_get_pointer(
    uint32 *pbo, int32 width, int32 height, int32 n_components
  );
#endif
#if 0
  void copy_texture_to_pbo(
    uint16 pbo_idx, unsigned char *data,
    int32 width, int32 height, int32 n_components
  );
#endif
#if 0
  uint32 generate_texture_from_pbo(
    uint32 *pbo, int32 width, int32 height, int32 n_components
  );
#endif
};

#endif
