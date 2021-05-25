#include "logs.hpp"
#include "behavior_functions.hpp"
#include "behavior.hpp"
#include "intrinsics.hpp"


BehaviorFunction behavior::function_map[(uint32)Behavior::length] = {
  (BehaviorFunction)nullptr,
  (BehaviorFunction)behavior_functions::test,
  (BehaviorFunction)behavior_functions::char_movement_test,
};


const char* behavior::behavior_to_string(Behavior behavior) {
  if (behavior == Behavior::none) {
    return "none";
  } else if (behavior == Behavior::test) {
    return "test";
  } else if (behavior == Behavior::char_movement_test) {
    return "char_movement_test";
  } else {
    logs::error("Don't know how to convert Behavior to string: %d", behavior);
    return "<unknown>";
  }
}


Behavior behavior::behavior_from_string(const char *str) {
  if (strcmp(str, "none") == 0) {
    return Behavior::none;
  } else if (strcmp(str, "test") == 0) {
    return Behavior::test;
  } else if (strcmp(str, "char_movement_test") == 0) {
    return Behavior::char_movement_test;
  } else {
    logs::fatal("Could not parse Behavior: %s", str);
    return Behavior::none;
  }
}


bool32 behavior::is_behavior_component_valid(BehaviorComponent *behavior_component) {
  return behavior_component->behavior != Behavior::none;
}


void behavior::update_behavior_components(
  void *state,
  BehaviorComponentSet *behavior_component_set,
  SpatialComponentSet *spatial_component_set,
  real64 t
) {
  each (behavior_component, behavior_component_set->components) {
    if (!is_behavior_component_valid(behavior_component)) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    auto handler = function_map[(uint32)behavior_component->behavior];
    if (handler) {
      handler(state, entity_handle);
    }
  }
}
