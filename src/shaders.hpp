#pragma once

namespace shaders {
  enum class ShaderType {none, standard, depth};

  struct ShaderAsset {
    char name[MAX_DEBUG_NAME_LENGTH];
    char vert_path[MAX_PATH];
    char frag_path[MAX_PATH];
    char geom_path[MAX_PATH];
    uint32 program;
    ShaderType type;
    uint32 n_texture_units;
    uint32 texture_units[MAX_N_TEXTURE_UNITS];
    GLenum texture_unit_types[MAX_N_TEXTURE_UNITS];
    bool did_set_texture_uniforms;

    /*
    Intrinsic uniform: A uniform declared by a shader. We only care
      about intrinsic uniforms in the Shader struct.

    Active uniforms: A uniform used in a shader, which can be either
      an intrinsic uniform, or a uniform from another source such as a
      uniform buffer object.
    */

    uint32 n_intrinsic_uniforms;
    int32 intrinsic_uniform_locations[MAX_N_UNIFORMS];
    char intrinsic_uniform_names[MAX_UNIFORM_NAME_LENGTH][MAX_N_UNIFORMS];
  };

  const char* shader_type_to_string(ShaderType shader_type);
  ShaderType shader_type_from_string(const char* str);
  bool32 is_shader_asset_valid(ShaderAsset *shader_asset);
  void set_int(ShaderAsset *shader_asset, const char *uniform_name, uint32 value);
  void set_bool(ShaderAsset *shader_asset, const char *uniform_name, bool value);
  void set_float(ShaderAsset *shader_asset, const char *uniform_name, float value);
  void set_vec2(ShaderAsset *shader_asset, const char *uniform_name, v2 *value);
  void set_vec3(ShaderAsset *shader_asset, const char *uniform_name, v3 *value);
  void set_vec4(ShaderAsset *shader_asset, const char *uniform_name, v4 *value);
  void set_mat2(ShaderAsset *shader_asset, const char *uniform_name, m2 *mat);
  void set_mat3(ShaderAsset *shader_asset, const char *uniform_name, m3 *mat);
  void set_mat4_multiple(
    ShaderAsset *shader_asset, uint32 n, const char *uniform_name, m4 *mat
  );
  void set_mat4(ShaderAsset *shader_asset, const char *uniform_name, m4 *mat);
  void reset_texture_units(ShaderAsset *shader_asset);
  uint32 add_texture_unit(
    ShaderAsset *shader_asset,
    uint32 new_texture_unit,
    GLenum new_texture_unit_type
  );
  void load_shader_asset(ShaderAsset *shader_asset, MemoryPool *memory_pool);
  ShaderAsset* init_shader_asset(
    ShaderAsset *shader_asset,
    MemoryPool *memory_pool,
    const char *new_name, ShaderType new_type,
    const char *vert_path, const char *frag_path, const char *geom_path
  );
  void destroy_shader_asset(ShaderAsset *shader_asset);
}

using shaders::ShaderType, shaders::ShaderAsset;
