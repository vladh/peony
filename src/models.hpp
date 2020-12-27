#ifndef MODELS_HPP
#define MODELS_HPP

namespace Models {
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
    Textures::Material *material;
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

  struct ModelAsset {
    const char *name;
    ModelSource model_source;
    char path[256]; // TODO: Fix unsafe strings?
    Array<Mesh> meshes;
    Array<Textures::Material> materials;
    EntityHandle entity_handle;
    SpatialComponent spatial_component;
    LightComponent light_component;
    BehaviorComponent behavior_component;
    Renderer::RenderPassFlag render_pass;
    bool32 is_mesh_data_loading_in_progress = false;
    bool32 is_texture_copying_to_pbo_done = false;
    bool32 is_texture_copying_to_pbo_in_progress = false;
    bool32 is_mesh_data_loading_done = false;
    bool32 is_shader_setting_done = false;
    bool32 is_texture_creation_done = false;
    bool32 is_vertex_buffer_setup_done = false;
    bool32 is_entity_creation_done = false;
  };

  void setup_mesh_vertex_buffers_for_data_source(
    Models::Mesh *mesh,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices
  );
  void setup_mesh_vertex_buffers_for_file_source(
    Models::Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
  );
  void load_mesh(
    Models::Mesh *mesh, Memory *memory, aiMesh *mesh_data, const aiScene *scene,
    glm::mat4 transform, Pack indices_pack
  );
  void load_node(
    Models::ModelAsset *model_asset,
    Memory *memory, aiNode *node, const aiScene *scene,
    glm::mat4 accumulated_transform, Pack indices_pack
  );
  void load_model_asset(
    Models::ModelAsset *model_asset, Memory *memory
  );
  void copy_textures_to_pbo(
    Models::ModelAsset *model_asset,
    Textures::PersistentPbo *persistent_pbo
  );
  void bind_texture_uniforms_for_mesh(Models::Mesh *mesh);
  void create_entities(
    Models::ModelAsset *model_asset,
    EntityManager *entity_manager,
    DrawableComponentManager *drawable_component_manager,
    SpatialComponentManager *spatial_component_manager,
    LightComponentManager *light_component_manager,
    BehaviorComponentManager *behavior_component_manager
  );
  void prepare_for_draw(
    Models::ModelAsset *model_asset,
    Memory *memory,
    Textures::PersistentPbo *persistent_pbo,
    Textures::TextureNamePool *texture_name_pool,
    Queue<Tasks::Task> *task_queue,
    EntityManager *entity_manager,
    DrawableComponentManager *drawable_component_manager,
    SpatialComponentManager *spatial_component_manager,
    LightComponentManager *light_component_manager,
    BehaviorComponentManager *behavior_component_manager
  );
  Models::ModelAsset* init_model_asset(
    ModelAsset *model_asset,
    Memory *memory,
    ModelSource model_source,
    const char *name,
    const char *path,
    Renderer::RenderPassFlag render_pass,
    EntityHandle entity_handle
  );
  Models::ModelAsset* init_model_asset(
    ModelAsset *model_asset,
    Memory *memory,
    ModelSource model_source,
    real32 *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices,
    const char *name,
    GLenum mode,
    Renderer::RenderPassFlag render_pass,
    EntityHandle entity_handle
  );
}

#endif
