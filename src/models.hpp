#ifndef MODELS_H
#define MODELS_H

#define MAX_N_MESHES 2048

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

struct Mesh {
  Array<Vertex> vertices;
  Array<uint32> indices;
  glm::mat4 transform;
  uint64 indices_pack;

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

  bool32 is_screenquad;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  GLenum mode;
};

struct Model {
  Array<Mesh> meshes;
#if 0
  bool32 should_load_textures_from_file;
#endif
  const char *directory;
};

uint32 models_load_texture_from_file(const char *path);
void models_setup_mesh(Mesh *mesh);
void models_init_mesh(Mesh *mesh, uint32 mode);
void models_load_mesh(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
);
void models_draw_mesh(Mesh *mesh, uint32 shader_program);
void models_draw_model(Model *model, uint32 shader_program);
#if 0
void models_load_mesh_textures(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
);
#endif
void models_load_model(
  Memory *memory, Model *model,
  const char *directory, const char *filename
);
ModelAsset* models_make_asset_from_file(
  Memory *memory, ModelAsset *model_asset,
  const char *name, const char *directory, const char *filename
);
void models_set_is_screenquad(Model *model);
ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode
);
void models_add_texture(
  Model *model, TextureType type, uint32 texture
);
void models_add_texture(
  Model *model, uint32 idx_mesh, TextureType type, uint32 texture
);
void models_add_texture_for_node_idx(
  Model *model, TextureType type, uint32 texture, uint8 node_depth, uint8 node_idx
);
void models_set_static_pbr(
  Model *model,
  glm::vec4 albedo, real32 metallic, real32 roughness, real32 ao
);
void models_set_static_pbr(
  Model *model, uint32 idx_mesh,
  glm::vec4 albedo, real32 metallic, real32 roughness, real32 ao
);

#endif
