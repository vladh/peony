#ifndef SHADER_H
#define SHADER_H

#define MAX_N_UNIFORMS 256

enum Uniform {
  UNIFORM_UNKNOWN,

  UNIFORM_ALBEDO_STATIC,
  UNIFORM_ALBEDO_TEXTURE,
  UNIFORM_AO_STATIC,
  UNIFORM_AO_TEXTURE,
  UNIFORM_DEPTH_TEXTURE_0,
  UNIFORM_DEPTH_TEXTURE_1,
  UNIFORM_DEPTH_TEXTURE_2,
  UNIFORM_DEPTH_TEXTURE_3,
  UNIFORM_DEPTH_TEXTURE_4,
  UNIFORM_DEPTH_TEXTURE_5,
  UNIFORM_DEPTH_TEXTURE_6,
  UNIFORM_DEPTH_TEXTURE_7,
  UNIFORM_EXPOSURE,
  UNIFORM_G_ALBEDO_TEXTURE,
  UNIFORM_G_NORMAL_TEXTURE,
  UNIFORM_G_PBR_TEXTURE,
  UNIFORM_G_POSITION_TEXTURE,
  UNIFORM_MESH_TRASNFORM,
  UNIFORM_METALLIC_STATIC,
  UNIFORM_METALLIC_TEXTURE,
  UNIFORM_MODEL,
  UNIFORM_NORMAL_TEXTURE,
  UNIFORM_N_DEPTH_TEXTURES,
  UNIFORM_ROUGHNESS_STATIC,
  UNIFORM_ROUGHNESS_TEXTURE,
  UNIFORM_SHADOW_LIGHT_IDX,
  UNIFORM_SHOULD_USE_NORMAL_MAP,
  UNIFORM_TEXT_COLOR,
  UNIFORM_TEXT_PROJECTION,
};

const uint16 DEPTH_TEXTURE_UNIFORMS[MAX_N_SHADOW_FRAMEBUFFERS] = {
  UNIFORM_DEPTH_TEXTURE_0,
  UNIFORM_DEPTH_TEXTURE_1,
  UNIFORM_DEPTH_TEXTURE_2,
  UNIFORM_DEPTH_TEXTURE_3,
  UNIFORM_DEPTH_TEXTURE_4,
  UNIFORM_DEPTH_TEXTURE_5,
  UNIFORM_DEPTH_TEXTURE_6,
  UNIFORM_DEPTH_TEXTURE_7,
};

struct Shader {
  uint32 program;
  int32 uniform_locations[MAX_N_UNIFORMS] = {0};
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
