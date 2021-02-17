LightComponentManager::LightComponentManager(
  Array<Entities::LightComponent> *new_components
) {
  this->components = new_components;
}


Entities::LightComponent* LightComponentManager::add(
  Entities::LightComponent light_component
) {
  assert(light_component.entity_handle != Entities::Entity::no_entity_handle);
  Entities::LightComponent *new_component = this->components->get(
    light_component.entity_handle
  );
  *new_component = light_component;
  return new_component;
}


Entities::LightComponent* LightComponentManager::add(
  Entities::EntityHandle entity_handle,
  Entities::LightType type,
  glm::vec3 direction,
  glm::vec4 color,
  glm::vec4 attenuation
) {
  assert(entity_handle != Entities::Entity::no_entity_handle);
  Entities::LightComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->type = type;
  new_component->direction = direction;
  new_component->color = color;
  new_component->attenuation = attenuation;
  return new_component;
}


Entities::LightComponent* LightComponentManager::get(Entities::EntityHandle handle) {
  if (handle == Entities::Entity::no_entity_handle) {
    return nullptr;
  }
  return this->components->get(handle);
}
