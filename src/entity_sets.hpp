#ifndef ENTITY_SETS_HPP
#define ENTITY_SETS_HPP

struct Mesh;

struct EntitySet {
  // NOTE: 0 is an invalid handle.
  Array<Entity> entities;
  EntityHandle last_handle;
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

namespace EntitySets {
  EntityHandle make_handle(
    EntitySet *entity_set
  );
  Entity* add_entity_to_set(
    EntitySet *entity_set,
    const char *debug_name
  );

  glm::mat4 make_model_matrix(
    SpatialComponentSet *spatial_component_set,
    SpatialComponent *spatial_component, ModelMatrixCache *cache
  );

  void draw(
    DrawableComponentSet *drawable_component_set,
    Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix
  );
  void draw_in_depth_mode(
    DrawableComponentSet *drawable_component_set,
    Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix,
    ShaderAsset *standard_depth_shader_asset
  );
  void draw_all(
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    RenderPassFlag render_pass,
    RenderMode render_mode,
    ShaderAsset *standard_depth_shader_asset
  );
}

#endif
