#ifndef LIGHT_COMPONENT_MANAGER_HPP
#define LIGHT_COMPONENT_MANAGER_HPP

class LightComponentManager {
public:
  Array<LightComponent> *components;

  LightComponent* add(
    LightComponent light_component
  );
  LightComponent* add(
    EntityHandle entity_handle,
    LightType type,
    glm::vec3 direction,
    glm::vec4 color,
    glm::vec4 attenuation
  );
  LightComponent* get(EntityHandle handle);
  LightComponentManager(
    Array<LightComponent> *components
  );
};

#endif
