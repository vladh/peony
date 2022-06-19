/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "logs.hpp"
#include "state.hpp"
#include "behavior_functions.hpp"


void behavior_functions::test(
  BehaviorState *behavior_state,
  EntityHandle entity_handle
) {
  State *state = behavior_state->state;
  EngineState *engine_state = &state->engine_state;

  SpatialComponent *spatial_component =
    engine_state->spatial_component_set.components[entity_handle];
  if (!spatial_component) {
    logs::error("Could not get SpatialComponent for BehaviorComponent");
    return;
  }

  spatial_component->rotation =
    glm::angleAxis((real32)sin(1.0f - (*engine::g_t)), v3(0.0f, 1.0f, 0.0f)) *
    glm::angleAxis((real32)cos(1.0f - (*engine::g_t)), v3(1.0f, 0.0f, 0.0f));
}


void behavior_functions::char_movement_test(
  BehaviorState *behavior_state,
  EntityHandle entity_handle
) {
  State *state = behavior_state->state;
  EngineState *engine_state = &state->engine_state;

  SpatialComponent *spatial_component =
    engine_state->spatial_component_set.components[entity_handle];
  if (!spatial_component) {
    logs::error("Could not get SpatialComponent for BehaviorComponent");
    return;
  }

  physics::Component *physics_component =
    engine_state->physics_component_set.components[entity_handle];
  if (!physics_component) {
    logs::error("Could not get physics::Component for BehaviorComponent");
    return;
  }
  Obb *obb = &physics_component->transformed_obb;

  // Update position
  spatial_component->position.x =
    (real32)sin((*engine::g_t) * 1.0f) * 4.0f +
    (real32)sin((*engine::g_t) * 2.0f) * 0.1f +
    (real32)cos((*engine::g_t) * 3.0f) * 0.3f;
  spatial_component->position.z =
    (real32)cos((*engine::g_t) * 1.0f) * 4.0f +
    (real32)cos((*engine::g_t) * 2.0f) * 0.3f +
    (real32)sin((*engine::g_t) * 3.0f) * 0.1f;
  spatial_component->rotation =
    glm::angleAxis(
      (real32)sin((*engine::g_t) * 3.0f) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)
    ) *
    glm::angleAxis(
      (real32)cos((*engine::g_t) * 2.0f) / 3.0f, v3(0.0f, 1.0f, 0.0f)
    ) *
    glm::angleAxis((real32)cos((*engine::g_t) * 2.0f), v3(1.0f, 0.0f, 0.0f)) *
    glm::angleAxis((real32)sin((*engine::g_t) * 1.5f) / 2.0f, v3(1.0f, 0.0f, 0.0f)) *
    glm::angleAxis((real32)sin((*engine::g_t) * 2.5f) / 1.5f, v3(0.5f, 0.5f, 0.2f));
  #if 0
  spatial_component->position.x = -5.0f;
  spatial_component->position.z = -5.0f;
  spatial_component->rotation =
    glm::angleAxis((real32)sin((*engine::g_t)) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)) *
    glm::angleAxis(radians(90.0f), v3(1.0f, 0.0f, 0.0f));
  #endif

  // Check collision with other entities
  {
    physics::CollisionManifold manifold = physics::find_physics_component_collision(
      physics_component,
      spatial_component,
      &engine_state->physics_component_set,
      &engine_state->spatial_component_set
    );

    if (manifold.did_collide) {
      v4 color;
      if (manifold.axis <= 5) {
        color = v4(1.0f, 0.0f, 0.0f, 1.0f);
      } else {
        color = v4(1.0f, 1.0f, 0.0f, 1.0f);
      }
      debugdraw::draw_obb(debugdraw::g_dds, obb, color);
      debugdraw::draw_obb(
        debugdraw::g_dds,
        &manifold.collidee->transformed_obb,
        color
      );
      debugdraw::draw_line(
        debugdraw::g_dds,
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
        debugdraw::g_dds,
        obb,
        v4(1.0f, 1.0f, 1.0f, 1.0f)
      );
    }
  }

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
      engine_state->physics_component_set
    );

    if (ray_collision_result.did_intersect) {
      debugdraw::draw_ray(
        debugdraw::g_dds,
        &ray,
        ray_collision_result.distance,
        v4(1.0f, 0.0f, 0.0f, 0.0f)
      );
      debugdraw::draw_obb(
        debugdraw::g_dds,
        &ray_collision_result.collidee->transformed_obb,
        v4(1.0f, 0.0f, 0.0f, 1.0f)
      );
    } else {
      debugdraw::draw_ray(
        debugdraw::g_dds,
        &ray,
        500.0f,
        v4(1.0f, 1.0f, 1.0f, 0.0f)
      );
    }
  }
  #endif
}
