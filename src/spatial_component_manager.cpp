SpatialComponentManager::SpatialComponentManager(
  Array<SpatialComponent> *new_components
) {
  this->components = new_components;
}


SpatialComponent* SpatialComponentManager::add(
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) {
  SpatialComponent *new_component = this->components->push();
  new_component->entity_handle = entity_handle;
  new_component->position = position;
  new_component->rotation = rotation;
  new_component->scale = scale;
  return new_component;
}


SpatialComponent* SpatialComponentManager::get(EntityHandle handle) {
  // NOTE: Normally we'd use a hash-map or something here, but
  // std::unordered_map is slow as heck. This nice ol' array is faster.
  // Let's look for something else if this starts showing up in the profiler.
  for (uint32 idx = 0; idx < this->components->get_size(); idx++) {
    if (this->components->get(idx)->entity_handle == handle) {
      return this->components->get(idx);
    }
  }
  return nullptr;
}
