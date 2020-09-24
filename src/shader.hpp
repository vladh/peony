#ifndef SHADER_H
#define SHADER_H

#define MAX_N_UNIFORMS 64
#define MAX_UNIFORM_NAME_LENGTH 32


enum UniformName {
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
  UNIFORM_MESH_TRANSFORM,
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

const char *UNIFORM_NAME_STRINGS[MAX_N_UNIFORMS] = {
  "unknown",
  "albedo_static",
  "albedo_texture",
  "ao_static",
  "ao_texture",
  "depth_textures[0]",
  "depth_textures[1]",
  "depth_textures[2]",
  "depth_textures[3]",
  "depth_textures[4]",
  "depth_textures[5]",
  "depth_textures[6]",
  "depth_textures[7]",
  "exposure",
  "g_albedo_texture",
  "g_normal_texture",
  "g_pbr_texture",
  "g_position_texture",
  "mesh_transform",
  "metallic_static",
  "metallic_texture",
  "model",
  "normal_texture",
  "n_depth_textures",
  "roughness_static",
  "roughness_texture",
  "shadow_light_idx",
  "should_use_normal_map",
  "text_color",
  "text_projection"
};

const UniformName DEPTH_TEXTURE_UNIFORM_NAMES[MAX_N_SHADOW_FRAMEBUFFERS] = {
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
void shader_set_int(Shader *shader, UniformName name, uint32 value);
void shader_set_bool(Shader *shader, UniformName name, bool value);
void shader_set_float(Shader *shader, UniformName name, float value);
void shader_set_vec2(Shader *shader, UniformName name, glm::vec2 *value);
void shader_set_vec3(Shader *shader, UniformName name, glm::vec3 *value);
void shader_set_vec4(Shader *shader, UniformName name, glm::vec4 *value);
void shader_set_mat2(Shader *shader, UniformName name, glm::mat2 *mat);
void shader_set_mat3(Shader *shader, UniformName name, glm::mat3 *mat);
void shader_set_mat4(Shader *shader, UniformName name, glm::mat4 *mat);

#endif
