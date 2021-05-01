namespace BehaviorFunctions {
  void test(State *state, EntityHandle entity_handle) {
    SpatialComponent *spatial_component =
      state->spatial_component_set.components[entity_handle];
    if (!spatial_component) {
      log_error("Could not get SpatialComponent for BehaviorComponent");
      return;
    }

    spatial_component->position = glm::vec3(
      (real32)sin(state->t) * 15.0f,
      (real32)((sin(state->t * 2.0f) + 1.5) * 3.0f),
      (real32)cos(state->t) * 15.0f
    );
  }

  void char_movement_test(State *state, EntityHandle entity_handle) {
    SpatialComponent *spatial_component =
      state->spatial_component_set.components[entity_handle];
    if (!spatial_component) {
      log_error("Could not get SpatialComponent for BehaviorComponent");
      return;
    }

    spatial_component->position.x = (real32)sin(state->t) * 10.0f;
    spatial_component->position.z = (real32)cos(state->t) * 10.0f;
    spatial_component->rotation.x = (real32)sin(state->t);
    spatial_component->rotation.y = (real32)cos(state->t);
  }
}

// NOTE: Must match Behavior enum
void (*BEHAVIOR_FUNCTION_MAP[(uint32)Behavior::length]) (
  State *state, EntityHandle entity_handle
) {
  nullptr,
  BehaviorFunctions::test,
  BehaviorFunctions::char_movement_test
};
