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

enum class ModelLoaderState {
  empty,
  initialized,
  mesh_data_being_loaded,
  mesh_data_loaded,
  vertex_buffers_set_up,
  complete
};

struct ModelLoader {
  ModelSource model_source;
  char model_path_or_builtin_model_name[MAX_PATH];
  Mesh meshes[MAX_N_MESHES];
  uint32 n_meshes;
  AnimationComponent animation_component;
  StackArray<char[MAX_TOKEN_LENGTH], MAX_N_PEONY_ARRAY_VALUES> material_names;
  ModelLoaderState state;
};

enum class EntityLoaderState {
  empty,
  initialized,
  complete
};

struct EntityLoader {
  char name[MAX_DEBUG_NAME_LENGTH];
  char model_path_or_builtin_model_name[MAX_PATH];
  EntityHandle entity_handle;
  SpatialComponent spatial_component;
  LightComponent light_component;
  BehaviorComponent behavior_component;
  RenderPassFlag render_pass;
  EntityLoaderState state;
};

struct DrawableComponent {
  EntityHandle entity_handle;
  Mesh mesh;
  RenderPassFlag target_render_pass = RenderPass::none;
};

namespace Models {
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
    ModelLoader *model_loader,
    glm::mat4 transform,
    Pack indices_pack
  );
  void destroy_mesh(Mesh *mesh);
  void load_node(
    ModelLoader *model_loader,
    aiNode *node, const aiScene *scene,
    glm::mat4 accumulated_transform, Pack indices_pack
  );
  void load_model_from_file(
    ModelLoader *model_loader,
    BoneMatrixPool *bone_matrix_pool
  );
  void load_model_from_data(
    ModelLoader *model_loader
  );
  bool32 prepare_model_loader_and_check_if_done(
    ModelLoader *model_loader,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool,
    Queue<Task> *task_queue,
    BoneMatrixPool *bone_matrix_pool
  );
  bool32 prepare_entity_loader_and_check_if_done(
    EntityLoader *entity_loader,
    EntitySet *entity_set,
    ModelLoader *model_loader,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    LightComponentSet *light_component_set,
    BehaviorComponentSet *behavior_component_set,
    AnimationComponentSet *animation_component_set
  );
  ModelLoader* init_model_loader(
    ModelLoader *model_loader,
    ModelSource model_source,
    const char *model_path_or_builtin_model_name
  );
  EntityLoader* init_entity_loader(
    EntityLoader *entity_loader,
    const char *name,
    const char *model_path_or_builtin_model_name,
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
