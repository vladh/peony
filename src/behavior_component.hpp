#ifndef BEHAVIOR_COMPONENT_HPP
#define BEHAVIOR_COMPONENT_HPP

enum class Behavior {
  none, test
};

class BehaviorComponent {
public:
  EntityHandle entity_handle = Entity::no_entity_handle;
  Behavior behavior = Behavior::none;

  bool32 is_valid() {
    return this->behavior != Behavior::none;
  }
  BehaviorComponent() {};
  BehaviorComponent(
    EntityHandle entity_handle,
    Behavior behavior
  ) :
    entity_handle(entity_handle),
    behavior(behavior)
  {
  }
};

#endif
