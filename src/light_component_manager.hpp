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
  Array<LightComponent> *components;
};

#endif
