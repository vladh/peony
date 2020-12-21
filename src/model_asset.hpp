#ifndef MODEL_ASSET_HPP
#define MODEL_ASSET_HPP

constexpr uint32 MAX_N_MESHES = 2048;
constexpr uint8 MAX_N_MATERIALS = 128;
constexpr char MODEL_DIR[] = "resources/models/";

// NOTE:
// * ModelSource::data: Loaded on initialisation, from given vertex data.
// * ModelSource::file: Loaded on demand, from file.
enum class ModelSource {file, data};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Mesh {
  glm::mat4 transform;
  Material *material;
  Pack indices_pack;
  uint32 n_vertices;
  uint32 n_indices;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  GLenum mode;
  Array<Vertex> vertices;
  Array<uint32> indices;
};

class ModelAsset {
public:
  const char *name;
  ModelSource model_source;
  char path[256]; // TODO: Fix unsafe strings?
  Array<Mesh> meshes;
  Array<Material> materials;
  EntityHandle entity_handle;
  SpatialComponent spatial_component;
  LightComponent light_component;
  BehaviorComponent behavior_component;
  RenderPass::Flag render_pass;
  bool32 is_mesh_data_loading_in_progress = false;
  bool32 is_texture_copying_to_pbo_done = false;
  bool32 is_texture_copying_to_pbo_in_progress = false;
  bool32 is_mesh_data_loading_done = false;
  bool32 is_shader_setting_done = false;
  bool32 is_texture_creation_done = false;
  bool32 is_vertex_buffer_setup_done = false;
  bool32 is_entity_creation_done = false;
  std::mutex mutex;
  static EntityManager *entity_manager;
  static DrawableComponentManager *drawable_component_manager;
  static SpatialComponentManager *spatial_component_manager;
  static LightComponentManager *light_component_manager;
  static BehaviorComponentManager *behavior_component_manager;

  void load(
    Memory *memory
  );
  void copy_textures_to_pbo(
    PersistentPbo *persistent_pbo
  );
  static ModelAsset* get_by_name(
    Array<ModelAsset> *assets, const char *name
  );
  void bind_texture_uniforms_for_mesh(
    Mesh *mesh
  );
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
    glm::mat4 accumulated_transform,
    Pack indices_pack
  );
  void create_entities();
  void prepare_for_draw(
    Memory *memory,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool,
    Queue<Task> *task_queue
  );
  ModelAsset(
    Memory *memory,
    ModelSource model_source,
    const char *name,
    const char *path,
    RenderPass::Flag render_pass,
    EntityHandle entity_handle
  );
  ModelAsset(
    Memory *memory,
    ModelSource model_source,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices,
    const char *name,
    GLenum mode,
    RenderPass::Flag render_pass,
    EntityHandle entity_handle
  );
};

#endif
