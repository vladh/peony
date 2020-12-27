#ifndef SHADERS_HPP
#define SHADERS_HPP

namespace Shaders {
  constexpr char SHADER_COMMON_PATH[] = "src/shaders/common.glsl";
  constexpr char SHADER_COMMON_FRAGMENT_PATH[] = "src/shaders/common_fragment.glsl";
  constexpr char SHADER_DIR[] = "src/shaders/";

  constexpr uint8 MAX_N_UNIFORMS = 64;
  constexpr uint8 MAX_UNIFORM_NAME_LENGTH = 64;
  constexpr uint8 MAX_N_TEXTURE_UNITS = 80;

  enum class ShaderType {none, standard, depth};

  struct ShaderAsset {
    const char *name;
    char vert_path[256]; // TODO: Fix unsafe strings?
    char frag_path[256]; // TODO: Fix unsafe strings?
    char geom_path[256]; // TODO: Fix unsafe strings?
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
  void assert_shader_status_ok(uint32 shader, const char* path);
  void assert_program_status_ok(uint32 program);
  uint32 make_shader(const char *path, const char *source, GLenum shader_type);
  uint32 make_program(uint32 vertex_shader, uint32 fragment_shader);
  uint32 make_program(
    uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
  );
  const char* load_file(Memory *memory, const char *path);
  const char* load_frag_file(Memory *memory, const char *path);
  int32 get_uniform_location(ShaderAsset *shader_asset, const char *name);
  void set_int(ShaderAsset *shader_asset, const char *name, uint32 value);
  void set_bool(ShaderAsset *shader_asset, const char *name, bool value);
  void set_float(ShaderAsset *shader_asset, const char *name, float value);
  void set_vec2(ShaderAsset *shader_asset, const char *name, glm::vec2 *value);
  void set_vec3(ShaderAsset *shader_asset, const char *name, glm::vec3 *value);
  void set_vec4(ShaderAsset *shader_asset, const char *name, glm::vec4 *value);
  void set_mat2(ShaderAsset *shader_asset, const char *name, glm::mat2 *mat);
  void set_mat3(ShaderAsset *shader_asset, const char *name, glm::mat3 *mat);
  void set_mat4(ShaderAsset *shader_asset, const char *name, glm::mat4 *mat);
  void reset_texture_units(ShaderAsset *shader_asset);
  uint32 add_texture_unit(
    ShaderAsset *shader_asset,
    uint32 texture_unit,
    GLenum texture_unit_type
  );
  void load_uniforms(ShaderAsset *shader_asset);
  void load_shader_asset(ShaderAsset *shader_asset, Memory *memory);
  ShaderAsset* init_shader_asset(
    ShaderAsset *shader_asset,
    Memory *memory, const char *name, ShaderType type,
    const char *vert_path, const char *frag_path, const char *geom_path
  );
}

#endif
