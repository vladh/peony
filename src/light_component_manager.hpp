#ifndef LIGHT_COMPONENT_MANAGER_H
#define LIGHT_COMPONENT_MANAGER_H

class LightComponentManager {
public:
  LightComponentManager(
    Array<LightComponent> *components
  );

  LightComponent* add(
    EntityHandle entity_handle,
    glm::vec4 color,
    glm::vec4 attenuation
  );

  LightComponent* get(EntityHandle handle);

private:
  // TODO: std::unordered_map is #slow, find a better implementation.
  // https://stackoverflow.com/questions/3300525/super-high-performance-c-c-hash-map-table-dictionary
  std::unordered_map<EntityHandle, LightComponent*> entity_handle_map;

  Array<LightComponent> *components;
};

#endif
