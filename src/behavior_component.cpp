bool32 BehaviorComponent::is_valid() {
  return this->behavior != Behavior::none;
}


BehaviorComponent::BehaviorComponent() {};


BehaviorComponent::BehaviorComponent(
  EntityHandle entity_handle,
  Behavior behavior
) :
  entity_handle(entity_handle),
  behavior(behavior)
{
}
