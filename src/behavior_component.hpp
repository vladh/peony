#ifndef BEHAVIOR_COMPONENT_HPP
#define BEHAVIOR_COMPONENT_HPP

enum class Behavior {
  none, test
};

class BehaviorComponent {
public:
  EntityHandle entity_handle = Entity::no_entity_handle;
  Behavior behavior = Behavior::none;

  bool32 is_valid();
  BehaviorComponent();
  BehaviorComponent(
    EntityHandle entity_handle,
    Behavior behavior
  );
};

#endif
