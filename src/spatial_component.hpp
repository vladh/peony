#ifndef SPATIAL_COMPONENT_HPP
#define SPATIAL_COMPONENT_HPP

class SpatialComponent {
public:
  EntityHandle entity_handle = Entity::no_entity_handle;
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat rotation = glm::angleAxis(0.0f, glm::vec3(0.0f));
  glm::vec3 scale = glm::vec3(0.0f);
  EntityHandle parent_entity_handle = Entity::no_entity_handle;

  bool32 has_dimensions();
  bool32 is_valid();
  SpatialComponent();
  SpatialComponent(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    EntityHandle parent_entity_handle
  );
  SpatialComponent(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );
};

#endif
