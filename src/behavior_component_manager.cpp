BehaviorComponentManager::BehaviorComponentManager(
  Array<BehaviorComponent> *new_components
) {
  this->components = new_components;
}


BehaviorComponent* BehaviorComponentManager::add(
  EntityHandle entity_handle,
  Behavior behavior
) {
  BehaviorComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->behavior = behavior;
  return new_component;
}


BehaviorComponent* BehaviorComponentManager::get(EntityHandle handle) {
  return this->components->get(handle);
}
