#ifndef TEXTURE_SET_ASSET_H
#define TEXTURE_SET_ASSET_H

class TextureSet {
public:
  bool32 have_textures_been_generated = false;
  bool32 is_screensize_dependent = false;

  Array<Texture> textures;
  Array<const char*> texture_uniform_names;

  // Hardcoded values for when we can't load a texture.
  glm::vec4 albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  real32 metallic_static = -1.0f;
  real32 roughness_static = -1.0f;
  real32 ao_static = -1.0f;
  bool32 should_use_normal_map = false;

  std::mutex mutex;

  TextureSet(
    Memory *memory
  ) :
    textures(Array<Texture>(&memory->entity_memory_pool, 16, "textures")),
    texture_uniform_names(Array<const char*>(&memory->entity_memory_pool, 16, "textures"))
  {
  };
  void add(Texture texture, const char *uniform_name);
  void set_albedo_static(glm::vec4 albedo_static);
  void set_metallic_static(real32 metallic_static);
  void set_roughness_static(real32 roughness_static);
  void set_ao_static(real32 ao_static);
  void copy_textures_to_pbo(
    PersistentPbo *persistent_pbo
  );
  void generate_textures_from_pbo(
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool
  );
};

#endif
