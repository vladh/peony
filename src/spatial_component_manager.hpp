#ifndef SPATIAL_COMPONENT_MANAGER_HPP
#define SPATIAL_COMPONENT_MANAGER_HPP

struct ModelMatrixCache {
  glm::mat4 last_model_matrix;
  SpatialComponent *last_model_matrix_spatial_component;
};

class SpatialComponentManager {
public:
  Array<SpatialComponent> *components;

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
  glm::mat4 SpatialComponentManager::make_model_matrix(
    SpatialComponent *spatial_component, ModelMatrixCache *cache
  );
  SpatialComponentManager(
    Array<SpatialComponent> *components
  );
};

#endif
