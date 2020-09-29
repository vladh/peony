#ifndef MODELS_H
#define MODELS_H

#define MAX_N_MESHES 2048
#define MAX_N_TEXTURE_SETS 8

struct ModelAsset;
struct Memory;

enum TextureType {
  TEXTURE_DIFFUSE, TEXTURE_SPECULAR, TEXTURE_DEPTH,
  TEXTURE_ALBEDO, TEXTURE_METALLIC, TEXTURE_ROUGHNESS, TEXTURE_AO,
  TEXTURE_NORMAL,
  TEXTURE_G_POSITION, TEXTURE_G_NORMAL, TEXTURE_G_ALBEDO, TEXTURE_G_PBR
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct TextureSet {
  // Valid `id` cannot be 0, as that is used for "unbound".
  // The `id` should be enough to distinguish between texture sets of
  // different models, so it should be more or less globally unique.
  uint32 id;

  uint32 n_depth_textures;
  uint32 depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

  // Loaded texture maps.
  uint32 albedo_texture;
  uint32 metallic_texture;
  uint32 roughness_texture;
  uint32 ao_texture;
  uint32 normal_texture;

  // Hardcoded values for when we can't load a texture.
  glm::vec4 albedo_static;
  real32 metallic_static;
  real32 roughness_static;
  real32 ao_static;

  // G-buffer used for lighting shader.
  uint32 g_position_texture;
  uint32 g_normal_texture;
  uint32 g_albedo_texture;
  uint32 g_pbr_texture;
};

struct Mesh {
  glm::mat4 transform;
  TextureSet *texture_set;
  ShaderAsset *shader_asset;
  uint64 indices_pack;
  uint32 n_vertices;
  uint32 n_indices;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  GLenum mode;
};

struct Model {
  Array<Mesh> meshes;
  Array<TextureSet> texture_sets;

  bool32 should_load_textures_from_file;
  const char *directory;
};


ModelAsset* models_make_asset_from_file(
  Memory *memory, ModelAsset *model_asset,
  const char *name, const char *directory, const char *filename
);

ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode
);

TextureSet* models_add_texture_set(Model *model);

void models_set_texture_set(Model *model, TextureSet *texture_set);

void models_set_texture_set_for_node_idx(
  Model *model, TextureSet *texture_set, uint8 node_depth, uint8 node_idx
);

void models_set_shader_asset_for_mesh(Model *model, uint32 idx_mesh, ShaderAsset *shader_asset);

void models_set_shader_asset(Model *model, ShaderAsset *shader_asset);

void models_set_shader_asset_for_node_idx(
  Model *model, ShaderAsset *shader_asset, uint8 node_depth, uint8 node_idx
);

void models_draw_model(
  ModelAsset *model_asset, glm::mat4 *model_matrix
);

void models_draw_model_in_depth_mode(
  ModelAsset *model_asset, glm::mat4 *model_matrix,
  ShaderAsset *entity_depth_shader_asset, uint32 shadow_light_idx
);

#endif
