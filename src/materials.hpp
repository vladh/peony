#ifndef TEXTURES_HPP
#define TEXTURES_HPP

constexpr uint32 MAX_N_TEXTURE_POOL_SIZES = 6;
constexpr uint32 MAX_UNIFORM_LENGTH = 256;

struct MaterialTemplate;
struct Task;

struct TextureNamePool {
  uint32 mipmap_max_level = 0;
  uint32 n_textures = 0;
  uint32 n_sizes = 0;
  uint32 sizes[MAX_N_TEXTURE_POOL_SIZES] = {0};
  uint32 idx_next[MAX_N_TEXTURE_POOL_SIZES] = {0};
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
  char path[MAX_PATH];
  uint32 texture_name = 0;
  int32 width = 0;
  int32 height = 0;
  int32 n_components = 0;
  uint16 pbo_idx_for_copy;
  bool32 is_screensize_dependent = false;
  // ASSUMPTION: A builtin texture can belong to multiple materials,
  // but a non-builtin texture can belong to only one material.
  // If we delete a material, we delete all its non-builtin textures.
  bool32 is_builtin = false;
};

struct BuiltinTextures {
  Texture *g_position_texture;
  Texture *g_normal_texture;
  Texture *g_albedo_texture;
  Texture *g_pbr_texture;

  Texture *l_color_texture;
  Texture *l_bright_color_texture;
  Texture *l_depth_texture;

  Texture *blur1_texture;
  Texture *blur2_texture;

  Texture *cube_shadowmaps_texture;
  Texture *texture_shadowmaps_texture;

  uint32 g_buffer;
  uint32 l_buffer;
  uint32 blur1_buffer;
  uint32 blur2_buffer;
  uint32 cube_shadowmaps_framebuffer;
  uint32 cube_shadowmaps;
  uint32 texture_shadowmaps_framebuffer;
  uint32 texture_shadowmaps;
  uint32 cube_shadowmap_width;
  uint32 cube_shadowmap_height;
  uint32 texture_shadowmap_width;
  uint32 texture_shadowmap_height;
  real32 shadowmap_near_clip_dist;
  real32 shadowmap_far_clip_dist;
};

enum class MaterialState {
  empty,
  initialized,
  textures_being_copied_to_pbo,
  textures_copied_to_pbo,
  complete
};

struct Material {
  char name[MAX_TOKEN_LENGTH];
  MaterialState state;
  bool32 have_textures_been_generated = false;
  bool32 is_screensize_dependent = false;
  ShaderAsset shader_asset;
  ShaderAsset depth_shader_asset;
  uint32 n_textures;
  Texture textures[MAX_N_TEXTURES_PER_MATERIAL];
  char texture_uniform_names[MAX_N_UNIFORMS][MAX_UNIFORM_LENGTH];
  uint32 idx_texture_uniform_names;

  // Hardcoded values for when we can't load a texture.
  glm::vec4 albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  real32 metallic_static = -1.0f;
  real32 roughness_static = -1.0f;
  real32 ao_static = -1.0f;
  bool32 should_use_normal_map = false;
};

namespace Materials {
  constexpr char TEXTURE_DIR[] = "resources/textures/";

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
  void destroy_texture(Texture *texture);
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
    const char *name
  );
  void destroy_material(Material *material);
  Material* get_material_by_name(
    Array<Material> *materials,
    const char *name
  );
  void add_texture_to_material(
    Material *material,
    Texture texture,
    const char *uniform_name
  );
  void copy_textures_to_pbo(
    Material *material,
    PersistentPbo *persistent_pbo
  );
  void generate_textures_from_pbo(
    Material *material,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool
  );
  void bind_texture_uniforms(Material *material);
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
    MemoryPool *memory_pool,
    uint32 n_textures,
    uint32 mipmap_max_level
  );
  uint32 get_new_texture_name(
    TextureNamePool *pool,
    uint32 target_size
  );
  const char* material_state_to_string(
    MaterialState material_state
  );
  bool32 prepare_material_and_check_if_done(
    Material *material,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool,
    Queue<Task> *task_queue
  );
  void create_material_from_template(
    Material *material,
    MaterialTemplate *material_template,
    BuiltinTextures *builtin_textures
  );
}

#endif
