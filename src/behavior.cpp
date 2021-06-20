/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "logs.hpp"
#include "behavior_functions.hpp"
#include "behavior.hpp"
#include "engine.hpp"
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
  BehaviorState *behavior_state,
  EngineState *engine_state
) {
  each (behavior_component, engine_state->behavior_component_set.components) {
    if (!is_behavior_component_valid(behavior_component)) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    auto handler = function_map[(uint32)behavior_component->behavior];
    if (handler) {
      handler(behavior_state, entity_handle);
    }
  }
}


void behavior::init(
  BehaviorState *behavior_state,
  State *state
) {
  // NOTE: behavior needs the global state to pass to the behavior functions
  behavior_state->state = state;
}
