#ifndef TEXTURE_SET_ASSET_H
#define TEXTURE_SET_ASSET_H

class TextureSetAsset : public Asset {
public:
  bool32 is_static;

  // Loaded texture maps.
  uint32 albedo_texture;
  uint32 metallic_texture;
  uint32 roughness_texture;
  uint32 ao_texture;
  uint32 normal_texture;

  const char *albedo_texture_path;
  const char *metallic_texture_path;
  const char *roughness_texture_path;
  const char *ao_texture_path;
  const char *normal_texture_path;

  // Hardcoded values for when we can't load a texture.
  glm::vec4 albedo_static;
  real32 metallic_static;
  real32 roughness_static;
  real32 ao_static;

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
  void load();
};

#endif
