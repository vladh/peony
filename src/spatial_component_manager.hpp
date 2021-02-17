#ifndef SPATIAL_COMPONENT_MANAGER_HPP
#define SPATIAL_COMPONENT_MANAGER_HPP

struct ModelMatrixCache {
  glm::mat4 last_model_matrix;
  Entities::SpatialComponent *last_model_matrix_spatial_component;
};

class SpatialComponentManager {
public:
  Array<Entities::SpatialComponent> *components;

  Entities::SpatialComponent* add(
    Entities::SpatialComponent spatial_component
  );
  Entities::SpatialComponent* add(
    Entities::EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    Entities::EntityHandle parent_entity_handle
  );
  Entities::SpatialComponent* add(
    Entities::EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );
  Entities::SpatialComponent* get(Entities::EntityHandle handle);
  glm::mat4 make_model_matrix(
    Entities::SpatialComponent *spatial_component, ModelMatrixCache *cache
  );
  SpatialComponentManager(
    Array<Entities::SpatialComponent> *components
  );
};

#endif
