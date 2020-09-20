#ifndef SHADER_H
#define SHADER_H

struct Shader {
  uint32 program;
};

struct ShaderAsset;

void shader_assert_shader_status_ok(uint32 shader);
void shader_assert_program_status_ok(uint32 shader);
uint32 shader_load(const char *source, GLenum shaderType);
uint32 shader_make_program(uint32 vertex_shader, uint32 fragment_shader);
uint32 shader_make_program(
  uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
);
uint32 shader_make_program_with_paths(
  Memory *memory, const char *vert_path, const char *frag_path
);
uint32 shader_make_program_with_paths(
  MemoryPool *memory, const char *vert_path, const char *frag_path, const char *geom_path
);
ShaderAsset* shader_make_asset(
  ShaderAsset *asset, const char *name,
  const char *vertex_path, const char *frag_path
);
ShaderAsset* shader_make_asset(
  ShaderAsset *asset, const char *name,
  const char *vertex_path, const char *frag_path, const char *geom_path
);
void shader_set_int(uint32 program, const char *name, uint32 value);
void shader_set_bool(uint32 program, const char *name, bool value);
void shader_set_float(uint32 program, const char *name, float value);
void shader_set_vec2(uint32 program, const char *name, glm::vec2 *value);
void shader_set_vec3(uint32 program, const char *name, glm::vec3 *value);
void shader_set_vec4(uint32 program, const char *name, glm::vec4 *value);
void shader_set_mat2(uint32 program, const char *name, glm::mat2 *mat);
void shader_set_mat3(uint32 program, const char *name, glm::mat3 *mat);
void shader_set_mat4(uint32 program, const char *name, glm::mat4 *mat);

#endif
