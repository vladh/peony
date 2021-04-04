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
  uint32 bone_idxs[MAX_N_BONES_PER_VERTEX];
  real32 bone_weights[MAX_N_BONES_PER_VERTEX];
};

struct Mesh {
  MemoryPool temp_memory_pool;
  glm::mat4 transform;
  char material_name[MAX_TOKEN_LENGTH];
  Pack indices_pack;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
  GLenum mode;
  Vertex *vertices;
  uint32 *indices;
  uint32 n_vertices;
  uint32 n_indices;
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
  Mesh meshes[MAX_N_MESHES];
  uint32 n_meshes;
  EntityHandle entity_handle;
  SpatialComponent spatial_component;
  LightComponent light_component;
  BehaviorComponent behavior_component;
  AnimationComponent animation_component;
  RenderPassFlag render_pass;
  EntityLoaderState state;
};

struct DrawableComponent {
  EntityHandle entity_handle;
  Mesh mesh;
  RenderPassFlag target_render_pass = RenderPass::none;
};

namespace Models {
  constexpr uint8 MAX_N_MATERIALS = 128;
  constexpr char MODEL_DIR[] = "resources/models/";

  void setup_mesh_vertex_buffers(
    Mesh *mesh,
    Vertex *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices
  );
  bool32 is_bone_only_node(aiNode *node);
  aiNode* find_root_bone(const aiScene *scene);
  void add_bone_tree_to_animation_component(
    AnimationComponent *animation_component,
    aiNode *node,
    uint32 idx_parent
  );
  void load_bones(
    AnimationComponent *animation_component,
    const aiScene *scene
  );
  void load_animations(
    AnimationComponent *animation_component,
    const aiScene *scene,
    BoneMatrixPool *bone_matrix_pool
  );
  void load_mesh(
    Mesh *mesh,
    aiMesh *mesh_data,
    const aiScene *scene,
    EntityLoader *entity_loader,
    glm::mat4 transform,
    Pack indices_pack
  );
  void destroy_mesh(Mesh *mesh);
  void load_node(
    EntityLoader *entity_loader,
    aiNode *node, const aiScene *scene,
    glm::mat4 accumulated_transform, Pack indices_pack
  );
  void load_model(
    EntityLoader *entity_loader,
    BoneMatrixPool *bone_matrix_pool
  );
  void create_entities(
    EntityLoader *entity_loader,
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    LightComponentSet *light_component_set,
    BehaviorComponentSet *behavior_component_set,
    AnimationComponentSet *animation_component_set
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
    BehaviorComponentSet *behavior_component_set,
    AnimationComponentSet *animation_component_set,
    BoneMatrixPool *bone_matrix_pool
  );
  EntityLoader* init_entity_loader(
    EntityLoader *entity_loader,
    ModelSource model_source,
    const char *name,
    const char *path,
    RenderPassFlag render_pass,
    EntityHandle entity_handle
  );
  EntityLoader* init_entity_loader(
    EntityLoader *entity_loader,
    ModelSource model_source,
    Vertex *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices,
    const char *name,
    GLenum mode,
    RenderPassFlag render_pass,
    EntityHandle entity_handle
  );
  bool32 is_mesh_valid(Mesh *mesh);
  bool32 is_drawable_component_valid(
    DrawableComponent *drawable_component
  );
  void destroy_drawable_component(
    DrawableComponent *drawable_component
  );
}

#endif
