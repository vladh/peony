SpatialComponentManager::SpatialComponentManager(
  Array<SpatialComponent> *components
) {
  this->components = components;
}


SpatialComponent* SpatialComponentManager::add(
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) {
  SpatialComponent *new_component = array_push<SpatialComponent>(components);
  new_component->entity_handle = entity_handle;
  new_component->position = position;
  new_component->rotation = rotation;
  new_component->scale = scale;
  entity_handle_map[entity_handle] = new_component;
  return new_component;
}


SpatialComponent* SpatialComponentManager::get(EntityHandle handle) {
  if (entity_handle_map.count(handle)) {
    return entity_handle_map[handle];
  } else {
    return nullptr;
  }
}
