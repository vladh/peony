#ifndef PHYSICS_HPP
#define PHYSICS_HPP

struct SpatialComponent;
struct PhysicsComponent;
struct SpatialComponentSet;
struct PhysicsComponentSet;

struct Obb {
  v3 center;
  v3 x_axis; // We can get the z axis with a cross product
  v3 y_axis; // We can get the z axis with a cross product
  v3 extents;
};

struct Face {
  v3 vertices[4];
};

struct CollisionManifold {
  bool32 did_collide;
  PhysicsComponent *collidee;
  // Axis of least penetration, so greatest separation
  real32 sep_max;
  uint32 axis;
  v3 normal;
};

struct Ray {
  v3 origin;
  v3 direction;
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

namespace Physics {
  constexpr real32 PARALLEL_FACE_TOLERANCE = 1.0e-2;
  Obb transform_obb(Obb obb, SpatialComponent *spatial_component);
  RaycastResult intersect_obb_ray(Obb *obb, Ray *ray);
  void update_manifold_for_face_axis(
    CollisionManifold *manifold,
    real32 sep, uint32 axis, v3 normal
  );
  void update_manifold_for_edge_axis(
    CollisionManifold *manifold,
    real32 sep, uint32 axis, v3 normal
  );
  v3 get_edge_contact_point(
    v3 a_edge_point,
    v3 a_axis,
    real32 a_axis_length,
    v3 b_edge_point,
    v3 b_axis,
    real32 b_axis_length,
    bool32 should_use_a_midpoint
  );
  Face get_incident_face(
    m3 *cob,
    v3 e,
    v3 c,
    v3 n
  );
  CollisionManifold intersect_obb_obb(
    Obb *a,
    Obb *b,
    SpatialComponent *spatial_a,
    SpatialComponent *spatial_b
  );
  CollisionManifold find_physics_component_collision(
    PhysicsComponent *self_physics,
    SpatialComponent *self_spatial,
    PhysicsComponentSet *physics_component_set,
    SpatialComponentSet *spatial_component_set
  );
  RayCollisionResult find_ray_collision(
    Ray *ray,
    PhysicsComponent *physics_component_to_ignore_or_nullptr,
    PhysicsComponentSet *physics_component_set
  );
}

#endif
