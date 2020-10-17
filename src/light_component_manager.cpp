LightComponentManager::LightComponentManager(
  Array<LightComponent> *new_components
) {
  this->components = new_components;
}


LightComponent* LightComponentManager::add(
  EntityHandle entity_handle,
  glm::vec3 direction,
  glm::vec4 color,
  glm::vec4 attenuation
) {
  LightComponent *new_component = this->components->push();
  new_component->entity_handle = entity_handle;
  new_component->direction = direction;
  new_component->color = color;
  new_component->attenuation = attenuation;
  return new_component;
}


LightComponent* LightComponentManager::get(EntityHandle handle) {
  // NOTE: Normally we'd use a hash-map or something here, but
  // std::unordered_map is slow as heck. This nice ol' array is faster.
  // Let's look for something else if this starts showing up in the profiler.
  for (uint32 idx = 0; idx < this->components->size; idx++) {
    if (this->components->get(idx)->entity_handle == handle) {
      return this->components->get(idx);
    }
  }
  return nullptr;
}
