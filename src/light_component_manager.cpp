LightComponentManager::LightComponentManager(
  Array<LightComponent> *new_components
) {
  this->components = new_components;
}


LightComponent* LightComponentManager::add(
  EntityHandle entity_handle,
  LightType type,
  glm::vec3 direction,
  glm::vec4 color,
  glm::vec4 attenuation
) {
  assert(entity_handle > 0);
  LightComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->type = type;
  new_component->direction = direction;
  new_component->color = color;
  new_component->attenuation = attenuation;
  return new_component;
}


LightComponent* LightComponentManager::get(EntityHandle handle) {
  assert(handle > 0);
  return this->components->get(handle);
}
