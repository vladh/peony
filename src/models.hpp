#ifndef MODELS_HPP
#define MODELS_HPP

enum class ModelSource {
  // Invalid.
  none,
  // Loaded on initialisation, from given vertex data.
  file,
  // Loaded on demand, from file.
  data
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Mesh {
  MemoryPool temp_memory_pool;
  glm::mat4 transform;
  char material_name[MAX_TOKEN_LENGTH];
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

enum class EntityLoaderState {
  empty,
  initialized,
  mesh_data_being_loaded,
  mesh_data_loaded,
  vertex_buffers_set_up,
  complete
};

struct EntityLoader {
  char name[MAX_DEBUG_NAME_LENGTH];
  ModelSource model_source;
  char path[MAX_PATH];
  char material_names[MAX_N_MATERIALS_PER_MODEL][MAX_TOKEN_LENGTH];
  uint32 n_materials;
  Array<Mesh> meshes;
  EntityHandle entity_handle;
  SpatialComponent spatial_component;
  LightComponent light_component;
  BehaviorComponent behavior_component;
  RenderPassFlag render_pass;
  EntityLoaderState state;
};

namespace Models {
  constexpr uint32 MAX_N_MESHES = 2048;
  constexpr uint8 MAX_N_MATERIALS = 128;
  constexpr char MODEL_DIR[] = "resources/models/";

  void setup_mesh_vertex_buffers_for_data_source(
    Mesh *mesh,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices
  );
  void setup_mesh_vertex_buffers_for_file_source(
    Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
  );
  void load_mesh(
    Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
    glm::mat4 transform, Pack indices_pack
  );
  void load_node(
    EntityLoader *entity_loader,
    aiNode *node, const aiScene *scene,
    glm::mat4 accumulated_transform, Pack indices_pack
  );
  void load_model(
    EntityLoader *entity_loader
  );
  void create_entities(
    EntityLoader *entity_loader,
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    LightComponentSet *light_component_set,
    BehaviorComponentSet *behavior_component_set
  );
  bool32 prepare_model_and_check_if_done(
    EntityLoader *entity_loader,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool,
    Queue<Task> *task_queue,
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    LightComponentSet *light_component_set,
    BehaviorComponentSet *behavior_component_set
  );
  EntityLoader* init_entity_loader(
    EntityLoader *entity_loader,
    MemoryPool *memory_pool,
    ModelSource model_source,
    const char *name,
    const char *path,
    RenderPassFlag render_pass,
    EntityHandle entity_handle
  );
  EntityLoader* init_entity_loader(
    EntityLoader *entity_loader,
    MemoryPool *memory_pool,
    ModelSource model_source,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices,
    const char *name,
    GLenum mode,
    RenderPassFlag render_pass,
    EntityHandle entity_handle
  );
}

#endif
