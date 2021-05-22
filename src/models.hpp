#pragma once

#include "types.hpp"
#include "stackarray.hpp"
#include "materials.hpp"
#include "entities.hpp"
#include "spatial.hpp"
#include "anim.hpp"
#include "behavior.hpp"
#include "physics.hpp"
#include "queue.hpp"
#include "tasks.hpp"
#include "pack.hpp"
#include "lights.hpp"

namespace models {
  enum class RenderMode {regular, depth};

  enum class RenderPass : uint32 {
    none = 0,
    shadowcaster = (1 << 0),
    deferred = (1 << 1),
    forward_depth = (1 << 2),
    forward_nodepth = (1 << 3),
    forward_skybox = (1 << 4),
    lighting = (1 << 5),
    postprocessing = (1 << 6),
    preblur = (1 << 7),
    blur1 = (1 << 8),
    blur2 = (1 << 9),
    renderdebug = (1 << 10),
  };

  enum class ModelSource {
    // Invalid.
    none,
    // Loaded on initialisation, from given vertex data.
    file,
    // Loaded on demand, from file.
    data
  };

  struct Vertex {
    v3 position;
    v3 normal;
    v2 tex_coords;
    uint32 bone_idxs[MAX_N_BONES_PER_VERTEX];
    real32 bone_weights[MAX_N_BONES_PER_VERTEX];
  };

  struct Mesh {
    MemoryPool temp_memory_pool;
    m4 transform;
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
    PhysicsComponent physics_component;
    RenderPass render_pass;
    EntityLoaderState state;
  };

  struct EntityLoaderSet {
    Array<EntityLoader> loaders;
  };

  struct DrawableComponent {
    EntityHandle entity_handle;
    Mesh mesh;
    RenderPass target_render_pass = RenderPass::none;
  };

  struct DrawableComponentSet {
    Array<DrawableComponent> components;
    uint32 last_drawn_shader_program;
  };

#include "models_data.hpp"

  const char* render_pass_to_string(RenderPass render_pass);
  RenderPass render_pass_from_string(const char* str);
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
    AnimationComponentSet *animation_component_set,
    PhysicsComponentSet *physics_component_set
  );
  bool32 is_model_loader_valid(ModelLoader *model_loader);
  bool32 is_entity_loader_valid(EntityLoader *entity_loader);
  ModelLoader* init_model_loader(
    ModelLoader *model_loader,
    ModelSource model_source,
    const char *model_path_or_builtin_model_name
  );
  EntityLoader* init_entity_loader(
    EntityLoader *entity_loader,
    const char *name,
    const char *model_path_or_builtin_model_name,
    RenderPass render_pass,
    EntityHandle entity_handle
  );
  bool32 is_drawable_component_valid(DrawableComponent *drawable_component);
  void destroy_drawable_component(DrawableComponent *drawable_component);
}

using models::ModelSource, models::Vertex, models::Mesh, models::ModelLoaderState,
  models::ModelLoader, models::EntityLoaderState, models::EntityLoader,
  models::EntityLoaderSet,
  models::DrawableComponent, models::DrawableComponentSet,
  models::RenderMode, models::RenderPass;
