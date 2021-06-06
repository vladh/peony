#pragma once

#include "../src_external/glad/glad.h"
#include "memory.hpp"
#include "queue.hpp"
#include "tasks.hpp"
#include "constants.hpp"
#include "shaders.hpp"
#include "array.hpp"
#include "types.hpp"

namespace materials {
  struct TextureNamePool {
    uint32 mipmap_max_level;
    uint32 n_textures;
    uint32 n_sizes;
    uint32 sizes[MAX_N_TEXTURE_POOL_SIZES];
    uint32 idx_next[MAX_N_TEXTURE_POOL_SIZES];
    uint32 *texture_names;
  };

  struct PersistentPbo {
    uint32 pbo;
    void *memory;
    int32 width;
    int32 height;
    int32 n_components;
    uint16 texture_count;
    uint32 texture_size;
    uint32 total_size;
    uint16 next_idx;
  };

  struct TextureAtlas {
    iv2 size;
    iv2 next_position;
    iv2 max_allocated_position_per_axis;
    uint32 texture_name;
  };

  enum class TextureType {
    none,
    albedo,
    metallic,
    roughness,
    ao,
    normal,
    shadowmaps_3d,
    shadowmaps_2d,
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

  enum class MaterialState {
    empty,
    initialized,
    textures_being_copied_to_pbo,
    textures_copied_to_pbo,
    complete
  };

  struct Material {
    char name[MAX_COMMON_NAME_LENGTH];
    MaterialState state;
    bool32 have_textures_been_generated;
    bool32 is_screensize_dependent;
    ShaderAsset shader_asset;
    ShaderAsset depth_shader_asset;
    uint32 n_textures;
    Texture textures[MAX_N_TEXTURES_PER_MATERIAL];
    char texture_uniform_names[MAX_N_UNIFORMS][MAX_UNIFORM_LENGTH];
    uint32 idx_texture_uniform_names;

    v4 albedo_static;
    real32 metallic_static;
    real32 roughness_static;
    real32 ao_static;
    bool32 should_use_normal_map;
  };

  struct MaterialsState {
    PersistentPbo persistent_pbo;
    TextureNamePool texture_name_pool;
    Array<Material> materials;
  };

  const char* texture_type_to_string(TextureType texture_type);
  TextureType texture_type_from_string(const char* str);
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
  TextureAtlas* init_texture_atlas(
    TextureAtlas* atlas,
    iv2 size
  );
  iv2 push_space_to_texture_atlas(
    TextureAtlas* atlas,
    iv2 space_size
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
    Material *material, Texture texture, const char *uniform_name
  );
  void bind_texture_uniforms(Material *material);
  void delete_persistent_pbo(PersistentPbo *ppbo);
  void init(
    MaterialsState *materials_state,
    MemoryPool *memory_pool
  );
  bool32 prepare_material_and_check_if_done(
    Material *material,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool,
    Queue<Task> *task_queue
  );
  void reload_shaders(Array<Material> *materials);
}

using materials::TextureNamePool, materials::PersistentPbo,
  materials::TextureAtlas, materials::TextureType, materials::Texture,
  materials::MaterialState, materials::Material,
  materials::MaterialsState;
