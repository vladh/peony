#ifndef ENTITY_SETS_HPP
#define ENTITY_SETS_HPP

struct Mesh;
struct DrawableComponent;

struct EntitySet {
  Array<Entity> entities;
  // The handle of the next entity which has not yet been created.
  // NOTE: 0 is an invalid handle.
  EntityHandle next_handle;
  // Certain entities at the start of our set are internal.
  // Remember the handle after we're done creating the internal entities,
  // so we can iterate through the non-internal ones, if we so desire.
  // This assumes all our internal entities will be contiguous and at the
  // start of our set.
  EntityHandle first_non_internal_handle;
};

struct EntityLoaderSet {
  Array<EntityLoader> loaders;
};

struct LightComponentSet {
  Array<LightComponent> components;
};

struct ModelMatrixCache {
  glm::mat4 last_model_matrix;
  SpatialComponent *last_model_matrix_spatial_component;
};

struct SpatialComponentSet {
  Array<SpatialComponent> components;
};

struct DrawableComponentSet {
  Array<DrawableComponent> components;
  uint32 last_drawn_shader_program;
};

struct BehaviorComponentSet {
  Array<BehaviorComponent> components;
};

struct AnimationComponentSet {
  Array<AnimationComponent> components;
};

namespace EntitySets {
  uint32 push_to_bone_matrix_pool(BoneMatrixPool *pool);
  glm::mat4* get_bone_matrix(
    BoneMatrixPool *pool,
    uint32 idx,
    uint32 idx_bone,
    uint32 idx_anim_key
  );
  real64* get_bone_matrix_time(
    BoneMatrixPool *pool,
    uint32 idx,
    uint32 idx_bone,
    uint32 idx_anim_key
  );
  EntityHandle make_handle(EntitySet *entity_set);
  Entity* add_entity_to_set(
    EntitySet *entity_set,
    const char *debug_name
  );
  glm::mat4 make_model_matrix(
    SpatialComponentSet *spatial_component_set,
    SpatialComponent *spatial_component, ModelMatrixCache *cache
  );
  void update_animation_components(
    AnimationComponentSet *animation_component_set,
    SpatialComponentSet *spatial_component_set,
    real64 t,
    BoneMatrixPool *bone_matrix_pool
  );
  void update_behavior_components(
    BehaviorComponentSet *behavior_component_set,
    SpatialComponentSet *spatial_component_set,
    real64 t
  );
  void update_light_components(
    LightComponentSet *light_component_set,
    SpatialComponentSet *spatial_component_set,
    real64 t,
    glm::vec3 camera_position,
    real32 dir_light_angle
  );
  uint32 get_bone_matrix_anim_key_for_timepoint(
    BoneMatrixPool *bone_matrix_pool,
    AnimationComponent *animation_component,
    real64 animation_timepoint,
    uint32 idx_bone_matrix_set,
    uint32 idx_bone
  );
  void make_bone_matrices_for_animation_bone(
    AnimationComponent *animation_component,
    aiNodeAnim *ai_channel,
    uint32 idx_animation,
    uint32 idx_bone,
    BoneMatrixPool *bone_matrix_pool
  );
  AnimationComponent* find_animation_component(
    SpatialComponent *spatial,
    SpatialComponentSet *spatial_component_set,
    AnimationComponentSet *animation_component_set
  );
  void draw(
    RenderMode render_mode,
    DrawableComponentSet *drawable_component_set,
    Mesh *mesh,
    Material *material,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix,
    glm::mat4 *bone_matrices,
    ShaderAsset *standard_depth_shader_asset
  );
  void draw_all(
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    AnimationComponentSet *animation_component_set,
    Array<Material> *materials,
    RenderPassFlag render_pass,
    RenderMode render_mode,
    ShaderAsset *standard_depth_shader_asset,
    real64 t
  );
}

#endif
