#ifndef SPATIAL_COMPONENT_MANAGER_H
#define SPATIAL_COMPONENT_MANAGER_H

class SpatialComponentManager {
public:
  SpatialComponentManager(
    Array<SpatialComponent> *components
  );

  SpatialComponent* add(
    EntityHandle entity_handle,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );

  SpatialComponent* get(EntityHandle handle);

private:
  // TODO: std::unordered_map is #slow, find a better implementation.
  // https://stackoverflow.com/questions/3300525/super-high-performance-c-c-hash-map-table-dictionary
  std::unordered_map<EntityHandle, SpatialComponent*> entity_handle_map;

  Array<SpatialComponent> *components;
};

#endif
