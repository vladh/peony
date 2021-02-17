#ifndef LIGHT_COMPONENT_MANAGER_HPP
#define LIGHT_COMPONENT_MANAGER_HPP

class LightComponentManager {
public:
  Array<Entities::LightComponent> *components;

  Entities::LightComponent* add(
    Entities::LightComponent light_component
  );
  Entities::LightComponent* add(
    Entities::EntityHandle entity_handle,
    Entities::LightType type,
    glm::vec3 direction,
    glm::vec4 color,
    glm::vec4 attenuation
  );
  Entities::LightComponent* get(Entities::EntityHandle handle);
  LightComponentManager(
    Array<Entities::LightComponent> *components
  );
};

#endif
