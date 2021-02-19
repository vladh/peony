#ifndef ENTITY_SETS_HPP
#define ENTITY_SETS_HPP

namespace Models {
  struct Mesh;
};

namespace EntitySets {
  struct EntitySet {
    // NOTE: 0 is an invalid handle.
    Entities::EntityHandle last_handle = 1;
    Array<Entities::Entity> *entities;
  };

  struct LightComponentSet {
    Array<Entities::LightComponent> *components;
  };

  struct ModelMatrixCache {
    glm::mat4 last_model_matrix;
    Entities::SpatialComponent *last_model_matrix_spatial_component;
  };

  struct SpatialComponentSet {
    Array<Entities::SpatialComponent> *components;
  };

  struct DrawableComponentSet {
    Array<Entities::DrawableComponent> *components;
    uint32 last_drawn_shader_program;
  };

  struct BehaviorComponentSet {
    Array<Entities::BehaviorComponent> *components;
  };

  Entities::EntityHandle make_handle(
    EntitySet *entity_set
  );
  Entities::Entity* get_entity_from_set(
    EntitySet *entity_set,
    Entities::EntityHandle handle
  );
  Entities::Entity* add_entity_to_set(
    EntitySet *entity_set,
    const char *debug_name
  );
  EntitySet* init_entity_set(
    EntitySet *entity_set,
    Array<Entities::Entity> *entities
  );

  Entities::LightComponent* get_light_component_from_set(
    LightComponentSet *light_component_set,
    Entities::EntityHandle handle
  );
  LightComponentSet* init_light_component_set(
    LightComponentSet *light_component_set,
    Array<Entities::LightComponent> *components
  );

  Entities::SpatialComponent* get_spatial_component_from_set(
    SpatialComponentSet *spatial_component_set,
    Entities::EntityHandle handle
  );
  glm::mat4 make_model_matrix(
    SpatialComponentSet *spatial_component_set,
    Entities::SpatialComponent *spatial_component, ModelMatrixCache *cache
  );
  SpatialComponentSet* init_spatial_component_set(
    SpatialComponentSet *spatial_component_set,
    Array<Entities::SpatialComponent> *components
  );

  Entities::DrawableComponent* get_drawable_component_from_set(
    DrawableComponentSet *drawable_component_set,
    Entities::EntityHandle handle
  );
  DrawableComponentSet* init_drawable_component_set(
    DrawableComponentSet *drawable_component_set,
    Array<Entities::DrawableComponent> *components
  );

  Entities::BehaviorComponent* get_behavior_component_from_set(
    BehaviorComponentSet *behavior_component_set,
    Entities::EntityHandle handle
  );
  BehaviorComponentSet* init_behavior_component_set(
    BehaviorComponentSet *behavior_component_set,
    Array<Entities::BehaviorComponent> *components
  );

  void draw(
    DrawableComponentSet *drawable_component_set,
    Models::Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix
  );
  void draw_in_depth_mode(
    DrawableComponentSet *drawable_component_set,
    Models::Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix,
    Shaders::ShaderAsset *standard_depth_shader_asset
  );
  void draw_all(
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    Renderer::RenderPassFlag render_pass,
    Renderer::RenderMode render_mode,
    Shaders::ShaderAsset *standard_depth_shader_asset
  );
}

#endif
