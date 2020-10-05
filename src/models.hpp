#ifndef MODELS_H
#define MODELS_H

constexpr uint32 MAX_N_MESHES = 2048;
constexpr uint8 MAX_N_TEXTURE_SETS = 8;
constexpr uint8 MAX_N_MESH_TEMPLATES = 128;

enum ModelSource {
  MODELSOURCE_FILE, MODELSOURCE_DATA
};

struct MeshShaderTextureTemplate {
  ShaderAsset *shader_asset;
  TextureSetAsset *texture_set_asset;
  bool32 apply_to_all_meshes;
  uint8 node_depth;
  uint8 node_idx;
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Mesh {
  glm::mat4 transform;
  TextureSetAsset *texture_set_asset;
  ShaderAsset *shader_asset;
  uint64 indices_pack;
  uint32 n_vertices;
  uint32 n_indices;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  GLenum mode;
  Array<Vertex> vertices;
  Array<uint32> indices;
};

class ModelAsset : public Asset {
public:
  // NOTE:
  // * MODELSOURCE_DATA: Loaded on initialisation, from given vertex data.
  // * MODELSOURCE_FILE: Loaded on demand, from file.
  ModelSource model_source;
  const char *directory;
  const char *filename;
  Array<Mesh> meshes;
  Array<TextureSetAsset> texture_sets;
  Array<MeshShaderTextureTemplate> mesh_templates;
  bool32 is_mesh_data_loading_in_progress = false;
  bool32 is_texture_preload_in_progress = false;
  bool32 is_texture_copying_to_pbo_done = false;
  bool32 is_texture_copying_to_pbo_in_progress = false;
  bool32 is_mesh_data_loading_done = false;
  bool32 is_shader_setting_done = false;
  bool32 is_texture_preload_done = false;
  bool32 is_texture_set_binding_done = false;
  bool32 is_vertex_buffer_setup_done = false;
  std::mutex mutex;

  ModelAsset(
    Memory *memory, ModelSource model_source,
    const char *name, const char *directory, const char *filename
  );
  ModelAsset(
    Memory *memory, ModelSource model_source,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices,
    const char *name,
    GLenum mode
  );
  void set_shader_to_mesh(
    uint32 idx_mesh, ShaderAsset *shader_asset
  );
  void set_shader(
    ShaderAsset *shader_asset
  );
  void set_shader_for_node_idx(
    ShaderAsset *shader_asset, uint8 node_depth, uint8 node_idx
  );
  void bind_texture_to_mesh(
    uint32 idx_mesh, TextureSetAsset *texture_set_asset
  );
  void bind_texture(
    TextureSetAsset *texture_set_asset
  );
  void bind_texture_for_node_idx(
    TextureSetAsset *texture_set_asset, uint8 node_depth, uint8 node_idx
  );
  void draw(
    Memory *memory, PersistentPbo *persistent_pbo,
    glm::mat4 *model_matrix
  );
  void draw_in_depth_mode(
    Memory *memory, PersistentPbo *persistent_pbo,
    glm::mat4 *model_matrix, ShaderAsset *standard_depth_shader_asset
  );
  void load(
    Memory *memory
  );
  void load_templates();
  static ModelAsset* get_by_name(
    Array<ModelAsset> *assets, const char *name
  );
  // Normal models use `::set_shader_asset()` and
  // `::bind_shader_and_texture()` and so on, but the screenquad
  // is initialised separately through this method, so as not to
  // polluate the normal pipeline with all the g_buffer/shadowmap
  // stuff.
  void bind_shader_and_texture_as_screenquad(
    uint32 g_position_texture, uint32 g_normal_texture,
    uint32 g_albedo_texture, uint32 g_pbr_texture,
    uint32 n_depth_textures,
    uint32 *depth_textures,
    ShaderAsset *shader_asset
  );

private:
  void setup_mesh_vertex_buffers_for_file_source(
    Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
  );
  void setup_mesh_vertex_buffers_for_data_source(
    Mesh *mesh,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices
  );
  void load_mesh(
    Memory *memory, Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
    glm::mat4 transform, Pack indices_pack
  );
  void load_node(
    Memory *memory, aiNode *node, const aiScene *scene,
    glm::mat4 accumulated_transform, Pack indices_pack
  );
  void bind_texture_uniforms_for_mesh(
    Mesh *mesh
  );
  void prepare_for_draw(
    Memory *memory, PersistentPbo *persistent_pbo
  );
  void preload_texture_set_image_data();
  void copy_texture_set_data_to_pbo(
    PersistentPbo *persistent_pbo
  );
};

#endif
