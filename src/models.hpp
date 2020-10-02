#ifndef MODELS_H
#define MODELS_H

#define MAX_N_MESHES 2048
#define MAX_N_TEXTURE_SETS 8

enum ModelSource {
  MODELSOURCE_FILE, MODELSOURCE_DATA
};

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
  glm::mat4 transform;
  TextureSetAsset *texture_set;
  ShaderAsset *shader_asset;
  uint64 indices_pack;
  uint32 n_vertices;
  uint32 n_indices;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  GLenum mode;
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
  void bind_shader_and_texture_to_mesh(
    uint32 idx_mesh, ShaderAsset *shader_asset, TextureSetAsset *texture_set
  );
  void bind_shader_and_texture(
    ShaderAsset *shader_asset, TextureSetAsset *texture_set
  );
  void bind_shader_and_texture_for_node_idx(
    ShaderAsset *shader_asset, TextureSetAsset *texture_set,
    uint8 node_depth, uint8 node_idx
  );
  void draw(
    Memory *memory, glm::mat4 *model_matrix
  );
  void draw_in_depth_mode(
    Memory *memory, glm::mat4 *model_matrix,
    ShaderAsset *entity_depth_shader_asset
  );
  void load(
    Memory *memory
  );
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
  void bind_texture_uniforms_for_mesh(Mesh *mesh);
};

#endif
