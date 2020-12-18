#ifndef SPATIAL_COMPONENT_MANAGER_HPP
#define SPATIAL_COMPONENT_MANAGER_HPP

class SpatialComponentManager {
public:
  Array<SpatialComponent> *components;
  static glm::mat4 last_model_matrix;
  static SpatialComponent *last_model_matrix_spatial_component;

  SpatialComponent* add(
    SpatialComponent spatial_component
  );

  SpatialComponent* add(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    EntityHandle parent_entity_handle
  );

  SpatialComponent* add(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );

  SpatialComponent* get(EntityHandle handle);

  glm::mat4 make_model_matrix(SpatialComponent *spatial_component);

  SpatialComponentManager(
    Array<SpatialComponent> *components
  );
};

#endif
