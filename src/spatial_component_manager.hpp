#ifndef SPATIAL_COMPONENT_MANAGER_H
#define SPATIAL_COMPONENT_MANAGER_H

class SpatialComponentManager {
public:
  Array<SpatialComponent> *components;

  SpatialComponentManager(
    Array<SpatialComponent> *components
  );

  SpatialComponent* add(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    SpatialComponent *parent
  );

  SpatialComponent* add(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );

  SpatialComponent* get(EntityHandle handle);
};

#endif
