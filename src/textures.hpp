#ifndef TEXTURES_HPP
#define TEXTURES_HPP

constexpr uint32 N_MAX_TEXTURE_POOL_SIZES = 6;
constexpr char TEXTURE_DIR[] = "resources/textures/";

namespace Textures {
  struct TextureNamePool {
    uint32 mipmap_max_level = 0;
    uint32 n_textures = 0;
    uint32 n_sizes = 0;
    uint32 sizes[N_MAX_TEXTURE_POOL_SIZES] = {0};
    uint32 idx_next[N_MAX_TEXTURE_POOL_SIZES] = {0};
    uint32 *texture_names = nullptr;
  };

  struct PersistentPbo {
    uint32 pbo = 0;
    void *memory = nullptr;
    int32 width = 0;
    int32 height = 0;
    int32 n_components = 0;
    uint16 texture_count = 0;
    uint32 texture_size = 0;
    uint32 total_size = 0;
    uint16 next_idx = 0;
    std::mutex mutex;
  };

  struct TextureAtlas {
    glm::ivec2 size;
    glm::ivec2 next_position;
    glm::ivec2 max_allocated_position_per_axis;
    uint32 texture_name;
  };

  enum class TextureType {
    none,
    albedo,
    metallic,
    roughness,
    ao,
    normal,
    shadowmap,
    other,

    // Screensize-dependent textures
    g_position,
    g_normal,
    g_albedo,
    g_pbr,
    l_color,
    l_bright_color,
    l_depth,
    blur1,
    blur2
  };

  struct Texture {
    GLenum target;
    TextureType type;
    char path[256]; // TODO: Fix unsafe strings?
    uint32 texture_name = 0;
    int32 width = 0;
    int32 height = 0;
    int32 n_components = 0;
    uint16 pbo_idx_for_copy;
    bool32 is_screensize_dependent = false;
  };

  struct Material {
    bool32 have_textures_been_generated = false;
    bool32 is_screensize_dependent = false;
    ShaderAsset *shader_asset;
    ShaderAsset *depth_shader_asset;
    Array<Texture> textures;
    Array<const char*> texture_uniform_names;

    // Hardcoded values for when we can't load a texture.
    glm::vec4 albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
    real32 metallic_static = -1.0f;
    real32 roughness_static = -1.0f;
    real32 ao_static = -1.0f;
    bool32 should_use_normal_map = false;
  };

  Texture* init_texture(
    Texture *texture,
    TextureType type,
    const char* path
  );
  Texture* init_texture(
    Texture *texture,
    GLenum target,
    TextureType type,
    uint32 texture_name,
    int32 width,
    int32 height,
    int32 n_components
  );
  bool32 is_texture_type_screensize_dependent(TextureType type);
  const char* texture_type_to_string(TextureType texture_type);
  TextureType texture_type_from_string(const char* str);
  TextureAtlas* init_texture_atlas(
    TextureAtlas* atlas,
    glm::ivec2 size
  );
  glm::ivec2 push_space_to_texture_atlas(
    TextureAtlas* atlas,
    glm::ivec2 space_size
  );
  Material* init_material(
    Material *material,
    Memory *memory
  );
  void add_texture_to_material(
    Material *material,
    Texture texture,
    const char *uniform_name
  );
  void copy_material_textures_to_pbo(
    Material *material,
    Textures::PersistentPbo *persistent_pbo
  );
  void generate_textures_from_pbo(
    Material *material,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool
  );
  PersistentPbo* init_persistent_pbo(
    PersistentPbo *ppbo,
    uint16 texture_count, int32 width, int32 height, int32 n_components
  );
  void delete_persistent_pbo(PersistentPbo *ppbo);
  uint16 get_new_persistent_pbo_idx(PersistentPbo *ppbo);
  void* get_offset_for_persistent_pbo_idx(
    PersistentPbo *ppbo, uint16 idx
  );
  void* get_memory_for_persistent_pbo_idx(
    PersistentPbo *pbbo, uint16 idx
  );
  TextureNamePool* init_texture_name_pool(
    TextureNamePool *pool,
    Memory *memory,
    uint32 n_textures,
    uint32 mipmap_max_level
  );
  uint32 get_new_texture_name(
    TextureNamePool *pool,
    uint32 target_size
  );
}

#endif
