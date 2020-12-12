#ifndef BEHAVIOR_COMPONENT_H
#define BEHAVIOR_COMPONENT_H

enum class Behavior {
  none, test
};

struct BehaviorComponent {
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
