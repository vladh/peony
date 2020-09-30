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
  return new_component;
}


LightComponent* LightComponentManager::get(EntityHandle handle) {
  // NOTE: Normally we'd use a hash-map or something here, but
  // std::unordered_map is slow as heck. This nice ol' array is faster.
  // Let's look for something else if this starts showing up in the profiler.
  for (uint32 idx = 0; idx < components->size; idx++) {
    if (components->items[idx].entity_handle == handle) {
      return &components->items[idx];
    }
  }
  return nullptr;
}