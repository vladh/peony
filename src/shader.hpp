#ifndef SHADER_H
#define SHADER_H

#define MAX_N_UNIFORMS 64
#define MAX_UNIFORM_NAME_LENGTH 64


struct Shader {
  uint32 program;
  uint32 n_intrinsic_uniforms;
  int32 intrinsic_uniform_locations[MAX_N_UNIFORMS];
  char intrinsic_uniform_names[MAX_UNIFORM_NAME_LENGTH][MAX_N_UNIFORMS];
};

struct ShaderAsset;

ShaderAsset* shader_make_asset(
  ShaderAsset *asset, const char *name,
  const char *vertex_path, const char *frag_path
);
ShaderAsset* shader_make_asset(
  ShaderAsset *asset, const char *name,
  const char *vertex_path, const char *frag_path, const char *geom_path
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
