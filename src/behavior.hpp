#pragma once

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

  typedef void (*BehaviorFunction) (void *state, EntityHandle entity_handle);

  BehaviorFunction function_map[(uint32)Behavior::length];

  const char* behavior_to_string(Behavior behavior);
  Behavior behavior_from_string(const char *str);
  bool32 is_behavior_component_valid(BehaviorComponent *behavior_component);
  void update_behavior_components(
    void *state,
    BehaviorComponentSet *behavior_component_set,
    SpatialComponentSet *spatial_component_set,
    real64 t
  );
}

using behavior::Behavior, behavior::BehaviorComponent,
  behavior::BehaviorComponentSet, behavior::BehaviorFunction;
