#ifndef BEHAVIOR_COMPONENT_HPP
#define BEHAVIOR_COMPONENT_HPP

enum class Behavior {
  none, test
};

const char* behavior_to_string(Behavior behavior);
Behavior behavior_from_string(const char *str);

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
