LightComponentManager::LightComponentManager(
  Array<LightComponent> *components
) {
  this->components = components;
}


LightComponent* LightComponentManager::add(
  EntityHandle entity_handle,
  glm::vec4 color,
  glm::vec4 attenuation
) {
  LightComponent *new_component = array_push<LightComponent>(components);
  new_component->entity_handle = entity_handle;
  new_component->color = color;
  new_component->attenuation = attenuation;
  entity_handle_map[entity_handle] = new_component;
  return new_component;
}


LightComponent* LightComponentManager::get(EntityHandle handle) {
  if (entity_handle_map.count(handle)) {
    return entity_handle_map[handle];
  } else {
    return nullptr;
  }
}
