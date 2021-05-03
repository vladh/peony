namespace BehaviorFunctions {
  void test(State *state, EntityHandle entity_handle) {
    SpatialComponent *spatial_component =
      state->spatial_component_set.components[entity_handle];
    if (!spatial_component) {
      log_error("Could not get SpatialComponent for BehaviorComponent");
      return;
    }

    spatial_component->position = v3(
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
#if 0
    spatial_component->position.x = (real32)sin(state->t * 1.0f) * 4.0f;
    spatial_component->position.z = (real32)cos(state->t * 1.0f) * 4.0f;
#endif
    spatial_component->position.x = -5.0f;
    spatial_component->position.z = -5.0f;
    spatial_component->rotation =
      glm::angleAxis((real32)sin(state->t) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)) *
      glm::angleAxis(radians(90.0f), v3(1.0f, 0.0f, 0.0f));

    // Check collision with other entities
#if 0
    {
      PhysicsComponent *collidee = Physics::find_physics_component_collision(
        physics_component, &state->physics_component_set
      );

      if (collidee) {
        DebugDraw::draw_obb(
          &state->debug_draw_state,
          obb,
          v4(1.0f, 0.0f, 0.0f, 1.0f)
        );
        DebugDraw::draw_obb(
          &state->debug_draw_state,
          &collidee->transformed_obb,
          v4(1.0f, 0.0f, 0.0f, 1.0f)
        );
      } else {
        DebugDraw::draw_obb(
          &state->debug_draw_state,
          obb,
          v4(1.0f, 1.0f, 1.0f, 1.0f)
        );
      }
    }
#endif

    // Check ray collision
    {
      Ray ray = {
        .origin = obb->center + obb->y_axis * obb->extents[1],
        .direction = obb->y_axis,
      };
      RayCollisionResult ray_collision_result = Physics::find_ray_collision(
        &ray,
        physics_component,
        &state->physics_component_set
      );

      if (ray_collision_result.did_intersect) {
        DebugDraw::draw_ray(
          &state->debug_draw_state,
          &ray,
          ray_collision_result.distance,
          v4(1.0f, 0.0f, 0.0f, 0.0f)
        );
        DebugDraw::draw_obb(
          &state->debug_draw_state,
          &ray_collision_result.collidee->transformed_obb,
          v4(1.0f, 0.0f, 0.0f, 1.0f)
        );
      } else {
        DebugDraw::draw_ray(
          &state->debug_draw_state,
          &ray,
          500.0f,
          v4(1.0f, 1.0f, 1.0f, 0.0f)
        );
      }
    }
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
