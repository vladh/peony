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

    PhysicsComponent *physics_component =
      state->physics_component_set.components[entity_handle];
    if (!physics_component) {
      log_error("Could not get PhysicsComponent for BehaviorComponent");
      return;
    }
    Obb *obb = &physics_component->transformed_obb;

    // Update position
    spatial_component->position.x = (real32)sin(state->t * 2.0f) * 5.0f;
    spatial_component->position.z = (real32)cos(state->t * 2.0f) * 5.0f;
    spatial_component->rotation.x = (real32)sin(state->t);
    spatial_component->rotation.y = (real32)cos(state->t);

    // Check collision with other entities
    PhysicsComponent *collidee = Physics::find_collision(
      physics_component, &state->physics_component_set
    );

    if (collidee) {
      DebugDraw::draw_obb(
        &state->debug_draw_state,
        obb,
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
      );
      DebugDraw::draw_obb(
        &state->debug_draw_state,
        &collidee->transformed_obb,
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
      );
    } else {
      DebugDraw::draw_obb(
        &state->debug_draw_state,
        obb,
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
      );
    }

    // Check ray collision
    Ray ray = {
      .origin = obb->center + obb->y_axis * obb->extents[1],
      .direction = obb->y_axis,
    };
    DebugDraw::draw_ray(
      &state->debug_draw_state, &ray, 5.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)
    );
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