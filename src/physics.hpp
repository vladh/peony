#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"

namespace physics {
  constexpr real32 PARALLEL_FACE_TOLERANCE = 1.0e-2;
  constexpr real32 RELATIVE_TOLERANCE = 1.00f;
  constexpr real32 ABSOLUTE_TOLERANCE = 0.10f;

  struct PhysicsComponent {
    EntityHandle entity_handle;
    Obb obb;
    Obb transformed_obb;
  };

  struct PhysicsComponentSet {
    Array<PhysicsComponent> components;
  };

  struct CollisionManifold {
    bool32 did_collide;
    PhysicsComponent *collidee;
    real32 sep_max;
    uint32 axis;
    v3 normal;
  };

  struct RaycastResult {
    bool32 did_intersect;
    real32 distance;
  };

  struct RayCollisionResult {
    bool32 did_intersect;
    real32 distance;
    PhysicsComponent *collidee;
  };

  RayCollisionResult find_ray_collision(
    Ray *ray,
    PhysicsComponent *physics_component_to_ignore_or_nullptr,
    PhysicsComponentSet *physics_component_set
  );
  CollisionManifold find_physics_component_collision(
    PhysicsComponent *self_physics,
    SpatialComponent *self_spatial,
    PhysicsComponentSet *physics_component_set,
    SpatialComponentSet *spatial_component_set
  );
  void update_physics_components(
    PhysicsComponentSet *physics_component_set,
    SpatialComponentSet *spatial_component_set
  );
}

using physics::RaycastResult, physics::RayCollisionResult, physics::CollisionManifold,
  physics::PhysicsComponent, physics::PhysicsComponentSet;
