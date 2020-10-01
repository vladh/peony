#ifndef MODELS_H
#define MODELS_H

#define MAX_N_MESHES 2048
#define MAX_N_TEXTURE_SETS 8

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

struct ModelAsset : Asset {
public:
  Array<Mesh> meshes;
  Array<TextureSet> texture_sets;
  const char *directory;
  const char *filename;

  ModelAsset(
    Memory *memory,
    const char *name, const char *directory, const char *filename
  );
  ModelAsset(
    Memory *memory,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices,
    const char *name,
    GLenum mode
  );
  TextureSet* create_texture_set();
  void bind_texture_set_to_mesh(TextureSet *texture_set);
  void bind_texture_set_to_mesh_for_node_idx(TextureSet *texture_set, uint8 node_depth, uint8 node_idx);
  void set_shader_asset_for_mesh(uint32 idx_mesh, ShaderAsset *shader_asset);
  void set_shader_asset(ShaderAsset *shader_asset);
  void set_shader_asset_for_node_idx(ShaderAsset *shader_asset, uint8 node_depth, uint8 node_idx);
  void draw(glm::mat4 *model_matrix);
  void draw_in_depth_mode(glm::mat4 *model_matrix, ShaderAsset *entity_depth_shader_asset);
  static ModelAsset* get_by_name(
    Array<ModelAsset> *assets, const char *name
  );

private:
  void setup_mesh_vertex_buffers(
    Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
  );
  void load_mesh(
    Memory *memory, Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
    glm::mat4 transform, Pack indices_pack
  );
  void load_node(
    Memory *memory, aiNode *node, const aiScene *scene,
    glm::mat4 accumulated_transform, Pack indices_pack
  );
  void load_model(Memory *memory);
  void init_texture_set(TextureSet *texture_set, uint32 id);
  void bind_texture_uniforms_for_mesh(Mesh *mesh);
};

#endif
