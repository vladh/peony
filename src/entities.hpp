#ifndef ENTITIES_HPP
#define ENTITIES_HPP

namespace Models {
  struct Mesh;
};

namespace Entities {
  typedef uint32 EntityHandle;

  struct Entity {
    EntityHandle handle;
    char debug_name[256]; // TODO: Fix unsafe strings.
    static EntityHandle no_entity_handle;
  };

  EntityHandle Entity::no_entity_handle = 0;

  enum class Behavior {
    none, test
  };

  struct BehaviorComponent {
    EntityHandle entity_handle = Entity::no_entity_handle;
    Behavior behavior = Behavior::none;
  };

  enum class LightType {none, point, directional};

  struct LightComponent {
    EntityHandle entity_handle = Entity::no_entity_handle;
    LightType type = LightType::none;
    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec4 color = glm::vec4(0.0f);
    glm::vec4 attenuation = glm::vec4(0.0f);
  };

  struct SpatialComponent {
    EntityHandle entity_handle = Entity::no_entity_handle;
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::angleAxis(0.0f, glm::vec3(0.0f));
    glm::vec3 scale = glm::vec3(0.0f);
    EntityHandle parent_entity_handle = Entity::no_entity_handle;
  };

  struct DrawableComponent {
    EntityHandle entity_handle = Entity::no_entity_handle;
    Models::Mesh *mesh = nullptr;
    Renderer::RenderPassFlag target_render_pass = Renderer::RenderPass::none;
  };

  const char* behavior_to_string(Behavior behavior);
  Behavior behavior_from_string(const char *str);
  bool32 is_behavior_component_valid(
    BehaviorComponent *behavior_component
  );
  BehaviorComponent* init_behavior_component(
    BehaviorComponent *behavior_component,
    EntityHandle entity_handle,
    Behavior behavior
  );

  const char* light_type_to_string(LightType light_type);
  LightType light_type_from_string(const char *str);
  uint32 light_type_to_int(LightType light_type);
  bool32 is_light_component_valid(
    LightComponent *light_component
  );
  LightComponent* init_light_component(
    LightComponent *light_component,
    EntityHandle entity_handle,
    LightType type,
    glm::vec3 direction,
    glm::vec4 color,
    glm::vec4 attenuation
  );

  void print_spatial_component(
    SpatialComponent *spatial_component
  );
  bool32 does_spatial_component_have_dimensions(
    SpatialComponent *spatial_component
  );
  bool32 is_spatial_component_valid(
    SpatialComponent *spatial_component
  );
  SpatialComponent* init_spatial_component(
    SpatialComponent *spatial_component,
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    EntityHandle parent_entity_handle
  );
  SpatialComponent* init_spatial_component(
    SpatialComponent *spatial_component,
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );

  bool32 is_drawable_component_valid(
    DrawableComponent *drawable_component
  );
  DrawableComponent* init_drawable_component(
    DrawableComponent *drawable_component,
    EntityHandle entity_handle,
    Models::Mesh *mesh,
    Renderer::RenderPassFlag target_render_pass
  );
}

#endif
