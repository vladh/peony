BehaviorComponentManager::BehaviorComponentManager(
  Array<BehaviorComponent> *new_components
) {
  this->components = new_components;
}


BehaviorComponent* BehaviorComponentManager::add(
  EntityHandle entity_handle,
  Behavior behavior
) {
  BehaviorComponent *new_component = this->components->push();
  new_component->entity_handle = entity_handle;
  new_component->behavior = behavior;
  return new_component;
}


BehaviorComponent* BehaviorComponentManager::get(EntityHandle handle) {
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
