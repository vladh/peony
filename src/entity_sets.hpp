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
  EntityHandle make_handle(EntitySet *entity_set);
  Entity* add_entity_to_set(
    EntitySet *entity_set,
    const char *debug_name
  );
  glm::mat4 make_model_matrix(
    SpatialComponentSet *spatial_component_set,
    SpatialComponent *spatial_component, ModelMatrixCache *cache
  );
  uint32 get_anim_channel_position_index_for_animation_timepoint(
    AnimChannel *anim_channel, real64 t
  );
  uint32 get_anim_channel_rotation_index_for_animation_timepoint(
    AnimChannel *anim_channel, real64 t
  );
  uint32 get_anim_channel_scaling_index_for_animation_timepoint(
    AnimChannel *anim_channel, real64 t
  );
  void make_bone_translation(
    AnimChannel *anim_channel, real64 t, glm::mat4 *translation
  );
  void make_bone_rotation(
    AnimChannel *anim_channel, real64 animation_timepoint, glm::mat4 *translation
  );
  void make_bone_scaling(
    AnimChannel *anim_channel, real64 animation_timepoint, glm::mat4 *scaling
  );
  void make_bone_matrices(
    glm::mat4 *local_bone_matrices,
    glm::mat4 *final_bone_matrices,
    AnimationComponent *animation_component,
    real64 t
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
    bool32 have_animations,
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
