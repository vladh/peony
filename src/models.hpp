#ifndef MODELS_HPP
#define MODELS_HPP

// NOTE:
// * ModelSource::none: Invalid.
// * ModelSource::data: Loaded on initialisation, from given vertex data.
// * ModelSource::file: Loaded on demand, from file.
enum class ModelSource {none, file, data};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Mesh {
  MemoryPool temp_memory_pool;
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

struct EntityLoader {
  char name[MAX_DEBUG_NAME_LENGTH];
  ModelSource model_source;
  char path[MAX_PATH];
  Array<Mesh> meshes;
  Array<Material> materials;
  EntityHandle entity_handle;
  SpatialComponent spatial_component;
  LightComponent light_component;
  BehaviorComponent behavior_component;
  RenderPassFlag render_pass;
  bool32 is_mesh_data_loading_in_progress = false;
  bool32 is_texture_copying_to_pbo_done = false;
  bool32 is_texture_copying_to_pbo_in_progress = false;
  bool32 is_mesh_data_loading_done = false;
  bool32 is_shader_setting_done = false;
  bool32 is_texture_creation_done = false;
  bool32 is_vertex_buffer_setup_done = false;
  bool32 is_entity_creation_done = false;
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
  void copy_textures_to_pbo(
    EntityLoader *entity_loader,
    PersistentPbo *persistent_pbo
  );
  void bind_texture_uniforms_for_mesh(Mesh *mesh);
  void create_entities(
    EntityLoader *entity_loader,
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    LightComponentSet *light_component_set,
    BehaviorComponentSet *behavior_component_set
  );
  bool prepare_for_draw(
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
