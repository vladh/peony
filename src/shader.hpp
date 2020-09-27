#ifndef SHADER_H
#define SHADER_H

constexpr uint8 MAX_N_UNIFORMS = 64;
constexpr uint8 MAX_UNIFORM_NAME_LENGTH = 64;
constexpr uint8 MAX_N_TEXTURE_UNITS = 80;
constexpr bool32 IS_NOT_FINDING_UNIFORM_FATAL = false;

enum ShaderType {
  SHADER_ENTITY,
  SHADER_ENTITY_DEPTH,
  SHADER_OTHER_OBJECT,
  SHADER_LIGHTING,
  SHADER_UI
};

struct Shader {
  uint32 program;
  uint32 n_texture_units;
  uint32 texture_units[MAX_N_TEXTURE_UNITS];
  GLenum texture_unit_types[MAX_N_TEXTURE_UNITS];
  bool did_set_texture_uniforms;
  ShaderType type;

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

struct ShaderAsset;

ShaderAsset* shader_make_asset(
  ShaderAsset *asset, Memory *memory, const char *name, ShaderType type,
  const char *vert_path, const char* frag_path
);
ShaderAsset* shader_make_asset(
  ShaderAsset *asset, Memory *memory, const char *name, ShaderType type,
  const char *vert_path, const char *frag_path, const char *geom_path
);
void shader_set_int(Shader *shader, const char *name, uint32 value);
void shader_set_bool(Shader *shader, const char *name, bool value);
void shader_set_float(Shader *shader, const char *name, float value);
void shader_set_vec2(Shader *shader, const char *name, glm::vec2 *value);
void shader_set_vec3(Shader *shader, const char *name, glm::vec3 *value);
void shader_set_vec4(Shader *shader, const char *name, glm::vec4 *value);
void shader_set_mat2(Shader *shader, const char *name, glm::mat2 *mat);
void shader_set_mat3(Shader *shader, const char *name, glm::mat3 *mat);
void shader_set_mat4(Shader *shader, const char *name, glm::mat4 *mat);

#endif
