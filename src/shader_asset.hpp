#ifndef SHADER_H
#define SHADER_H

#define SHADER_DIR "src/shaders/"
#define SHADER_COMMON_PATH SHADER_DIR"common.glsl"
#define SHADER_COMMON_FRAGMENT_PATH SHADER_DIR"common_fragment.glsl"

constexpr uint8 MAX_N_UNIFORMS = 64;
constexpr uint8 MAX_UNIFORM_NAME_LENGTH = 64;
constexpr uint8 MAX_N_TEXTURE_UNITS = 80;

enum class ShaderType {standard, depth};

class ShaderAsset {
public:
  const char *name;
  const char *vert_path;
  const char *frag_path;
  const char *geom_path;
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

  ShaderAsset(
    Memory *memory, const char *name, ShaderType type,
    const char *vert_path, const char *frag_path, const char *geom_path
  );
  void load(Memory *memory);
  void set_int(const char *name, uint32 value);
  void set_bool(const char *name, bool value);
  void set_float(const char *name, float value);
  void set_vec2(const char *name, glm::vec2 *value);
  void set_vec3(const char *name, glm::vec3 *value);
  void set_vec4(const char *name, glm::vec4 *value);
  void set_mat2(const char *name, glm::mat2 *mat);
  void set_mat3(const char *name, glm::mat3 *mat);
  void set_mat4(const char *name, glm::mat4 *mat);
  void reset_texture_units();
  uint32 add_texture_unit(
    uint32 texture_unit, GLenum texture_unit_type
  );
  static ShaderAsset* get_by_name(
    Array<ShaderAsset> *assets, const char *name
  );

private:
  void assert_shader_status_ok(uint32 shader, const char* path);
  void assert_program_status_ok(uint32 program);
  uint32 make_shader(const char *path, const char *source, GLenum shader_type);
  uint32 make_program(uint32 vertex_shader, uint32 fragment_shader);
  uint32 make_program(
    uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
  );
  const char* load_file(Memory *memory, const char *path);
  const char* load_frag_file(Memory *memory, const char *path);
  int32 get_uniform_location(const char *name);
  void load_uniforms();
};


#endif
