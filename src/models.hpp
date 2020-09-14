#ifndef MODELS_H
#define MODELS_H

struct ModelAsset;
struct Memory;

enum TextureSource {
  TEXTURE_NONE, TEXTURE_ID, TEXTURE_FILE
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Texture {
  uint32 id;
  const char *type;
  bool32 was_loaded_from_file;
  const char *filename;
};

struct Mesh {
  Array<Vertex> vertices;
  Array<uint32> indices;
  Array<Texture> textures;
  bool32 does_use_indices;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  uint32 mode;
};
ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  ShaderAsset *shader_asset,
  real32 *vertex_data, uint32 n_vertices,
  real32 *index_data, uint32 n_indices,
  const char *name,
  TextureSource texture_type, const char *texture_path, uint32 texture_id,
  uint32 mode
);

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
  real32 *index_data, uint32 n_indices,
  const char *name, const char *texture_path
);

#endif
