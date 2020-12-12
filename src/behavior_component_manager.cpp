BehaviorComponentManager::BehaviorComponentManager(
  Array<BehaviorComponent> *new_components
) {
  this->components = new_components;
}


BehaviorComponent* BehaviorComponentManager::add(
  BehaviorComponent behavior_component
) {
  assert(behavior_component.entity_handle != Entity::no_entity_handle);
  BehaviorComponent *new_component = this->components->get(
    behavior_component.entity_handle
  );
  *new_component = behavior_component;
  return new_component;
}


BehaviorComponent* BehaviorComponentManager::add(
  EntityHandle entity_handle,
  Behavior behavior
) {
  assert(entity_handle != Entity::no_entity_handle);
  BehaviorComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->behavior = behavior;
  return new_component;
}


BehaviorComponent* BehaviorComponentManager::get(EntityHandle handle) {
  if (handle == Entity::no_entity_handle) {
    return nullptr;
  }
  return this->components->get(handle);
}
