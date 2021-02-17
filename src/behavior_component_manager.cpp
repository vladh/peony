BehaviorComponentManager::BehaviorComponentManager(
  Array<Entities::BehaviorComponent> *new_components
) {
  this->components = new_components;
}


Entities::BehaviorComponent* BehaviorComponentManager::add(
  Entities::BehaviorComponent behavior_component
) {
  assert(behavior_component.entity_handle != Entities::Entity::no_entity_handle);
  Entities::BehaviorComponent *new_component = this->components->get(
    behavior_component.entity_handle
  );
  *new_component = behavior_component;
  return new_component;
}


Entities::BehaviorComponent* BehaviorComponentManager::add(
  Entities::EntityHandle entity_handle,
  Entities::Behavior behavior
) {
  assert(entity_handle != Entities::Entity::no_entity_handle);
  Entities::BehaviorComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->behavior = behavior;
  return new_component;
}


Entities::BehaviorComponent* BehaviorComponentManager::get(
  Entities::EntityHandle handle
) {
  if (handle == Entities::Entity::no_entity_handle) {
    return nullptr;
  }
  return this->components->get(handle);
}
