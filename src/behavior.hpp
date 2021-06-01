#pragma once

#include "types.hpp"
#include "array.hpp"
#include "entities.hpp"
#include "spatial.hpp"

struct State;
struct EngineState;

namespace behavior {
  enum class Behavior {
    none,
    test,
    char_movement_test,
    length
  };

  struct BehaviorComponent {
    EntityHandle entity_handle;
    Behavior behavior = Behavior::none;
  };

  struct BehaviorComponentSet {
    Array<BehaviorComponent> components;
  };

  struct BehaviorState {
    State *state;
  };

  typedef void (*BehaviorFunction) (void *state, EntityHandle entity_handle);

  extern BehaviorFunction function_map[(uint32)Behavior::length];

  const char* behavior_to_string(Behavior behavior);
  Behavior behavior_from_string(const char *str);
  bool32 is_behavior_component_valid(BehaviorComponent *behavior_component);
  void update_behavior_components(
    BehaviorState *behavior_state,
    EngineState *engine_state
  );
  void init(
    BehaviorState *behavior_state,
    State *state
  );
}

using behavior::Behavior, behavior::BehaviorComponent,
  behavior::BehaviorComponentSet, behavior::BehaviorFunction,
  behavior::BehaviorState;
