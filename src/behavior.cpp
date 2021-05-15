namespace behavior {
  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
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


  // -----------------------------------------------------------
  // Global variables
  // -----------------------------------------------------------
  BehaviorFunction function_map[(uint32)Behavior::length];


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  const char* behavior_to_string(Behavior behavior) {
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


  Behavior behavior_from_string(const char *str) {
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


  bool32 is_behavior_component_valid(
    BehaviorComponent *behavior_component
  ) {
    return behavior_component->behavior != Behavior::none;
  }


  void update_behavior_components(
    void *state,
    BehaviorComponentSet *behavior_component_set,
    SpatialComponentSet *spatial_component_set,
    real64 t
  ) {
    for_each (behavior_component, behavior_component_set->components) {
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
}

using behavior::Behavior, behavior::BehaviorComponent,
  behavior::BehaviorComponentSet, behavior::BehaviorFunction;
