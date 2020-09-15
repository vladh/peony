#ifndef MODELS_H
#define MODELS_H

struct ModelAsset;
struct Memory;

enum TextureType {
  TEXTURE_DIFFUSE, TEXTURE_SPECULAR, TEXTURE_DEPTH
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Mesh {
  Array<Vertex> vertices;
  Array<uint32> indices;

  uint32 n_diffuse_textures;
  uint32 diffuse_textures[16];
  uint32 n_specular_textures;
  uint32 specular_textures[16];
  uint32 n_depth_textures;
  uint32 depth_textures[16];

  bool32 does_use_indices;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  uint32 mode;
};

struct Model {
  Array<Mesh> meshes;
  ShaderAsset *shader_asset;
  bool32 should_load_textures_from_file;
  const char *directory;
};

uint32 models_load_texture_from_file(const char *path);
void models_setup_mesh(Mesh *mesh);
void models_load_mesh(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
);
void models_draw_mesh(Mesh *mesh, uint32 shader_program);
void models_draw_model(Model *model, uint32 shader_program);
void models_load_model_node(
  Memory *memory, Model *model,
  aiNode *node, const aiScene *scene
);
void models_load_model(
  Memory *memory, Model *model,
  const char *directory, const char *filename
);
ModelAsset* models_make_asset_from_file(
  Memory *memory, ModelAsset *model_asset, ShaderAsset *shader_asset,
  const char *name, const char *directory, const char *filename
);
ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  ShaderAsset *shader_asset,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  uint32 mode
);

#endif
