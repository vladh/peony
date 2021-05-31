#include "logs.hpp"
#include "behavior_functions.hpp"


void behavior_functions::test(State *state, EntityHandle entity_handle) {
  SpatialComponent *spatial_component =
    state->spatial_component_set.components[entity_handle];
  if (!spatial_component) {
    logs::error("Could not get SpatialComponent for BehaviorComponent");
    return;
  }

  spatial_component->rotation =
    glm::angleAxis((real32)sin(1.0f - state->t), v3(0.0f, 1.0f, 0.0f)) *
    glm::angleAxis((real32)cos(1.0f - state->t), v3(1.0f, 0.0f, 0.0f));
}


void behavior_functions::char_movement_test(State *state, EntityHandle entity_handle) {
  SpatialComponent *spatial_component =
    state->spatial_component_set.components[entity_handle];
  if (!spatial_component) {
    logs::error("Could not get SpatialComponent for BehaviorComponent");
    return;
  }

  PhysicsComponent *physics_component =
    state->physics_component_set.components[entity_handle];
  if (!physics_component) {
    logs::error("Could not get PhysicsComponent for BehaviorComponent");
    return;
  }
  Obb *obb = &physics_component->transformed_obb;

  // Update position
  #if 1
  spatial_component->position.x =
    (real32)sin(state->t * 1.0f) * 4.0f +
    (real32)sin(state->t * 2.0f) * 0.1f +
    (real32)cos(state->t * 3.0f) * 0.3f;
  spatial_component->position.z =
    (real32)cos(state->t * 1.0f) * 4.0f +
    (real32)cos(state->t * 2.0f) * 0.3f +
    (real32)sin(state->t * 3.0f) * 0.1f;
  spatial_component->rotation =
    glm::angleAxis((real32)sin(state->t * 3.0f) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)) *
    glm::angleAxis((real32)cos(state->t * 2.0f) / 3.0f, v3(0.0f, 1.0f, 0.0f)) *
    glm::angleAxis((real32)cos(state->t * 2.0f), v3(1.0f, 0.0f, 0.0f)) *
    glm::angleAxis((real32)sin(state->t * 1.5f) / 2.0f, v3(1.0f, 0.0f, 0.0f)) *
    glm::angleAxis((real32)sin(state->t * 2.5f) / 1.5f, v3(0.5f, 0.5f, 0.2f));
  #endif
  #if 0
  spatial_component->position.x = -5.0f;
  spatial_component->position.z = -5.0f;
  spatial_component->rotation =
    glm::angleAxis((real32)sin(state->t) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)) *
    glm::angleAxis(radians(90.0f), v3(1.0f, 0.0f, 0.0f));
  #endif

  // Check collision with other entities
  #if 1
  {
    CollisionManifold manifold = physics::find_physics_component_collision(
      physics_component,
      spatial_component,
      &state->physics_component_set,
      &state->spatial_component_set
    );

    if (manifold.did_collide) {
      v4 color;
      if (manifold.axis <= 5) {
        color = v4(1.0f, 0.0f, 0.0f, 1.0f);
      } else {
        color = v4(1.0f, 1.0f, 0.0f, 1.0f);
      }
      debugdraw::draw_obb(&state->debug_draw_state, obb, color);
      debugdraw::draw_obb(
        &state->debug_draw_state,
        &manifold.collidee->transformed_obb,
        color
      );
      debugdraw::draw_line(
        &state->debug_draw_state,
        obb->center,
        obb->center + manifold.normal * 100.0f,
        color
      );
      gui::log("manifold.axis = %d", manifold.axis);
      gui::log("manifold.sep_max = %f", manifold.sep_max);
      gui::log(
        "manifold.normal = (%f, %f, %f)",
        manifold.normal.x,
        manifold.normal.y,
        manifold.normal.z
      );
      gui::log("length(manifold.normal) = %f", length(manifold.normal));
      gui::log("---");
    } else {
      debugdraw::draw_obb(
        &state->debug_draw_state,
        obb,
        v4(1.0f, 1.0f, 1.0f, 1.0f)
      );
    }
  }
  #endif

  // Check ray collision
  #if 0
  {
    Ray ray = {
      .origin = obb->center + obb->y_axis * obb->extents[1],
      .direction = obb->y_axis,
    };
    RayCollisionResult ray_collision_result = physics::find_ray_collision(
      &ray,
      physics_component,
      &state->physics_component_set
    );

    if (ray_collision_result.did_intersect) {
      debugdraw::draw_ray(
        &state->debug_draw_state,
        &ray,
        ray_collision_result.distance,
        v4(1.0f, 0.0f, 0.0f, 0.0f)
      );
      debugdraw::draw_obb(
        &state->debug_draw_state,
        &ray_collision_result.collidee->transformed_obb,
        v4(1.0f, 0.0f, 0.0f, 1.0f)
      );
    } else {
      debugdraw::draw_ray(
        &state->debug_draw_state,
        &ray,
        500.0f,
        v4(1.0f, 1.0f, 1.0f, 0.0f)
      );
    }
  }
  #endif
}
