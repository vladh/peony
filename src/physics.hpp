#ifndef PHYSICS_HPP
#define PHYSICS_HPP

struct SpatialComponent;
struct PhysicsComponent;
struct PhysicsComponentSet;

struct Obb {
  v3 center;
  v3 x_axis; // We can get the z axis with a cross product
  v3 y_axis; // We can get the z axis with a cross product
  v3 extents;
};

struct Ray {
  v3 origin;
  v3 direction;
};

namespace Physics {
  Obb transform_obb(Obb obb, SpatialComponent *spatial_component);
  bool32 intersect_obb_ray(Obb *obb, Ray *ray);
  bool32 intersect_obb_obb(Obb *obb1, Obb *obb2);
  PhysicsComponent* find_physics_component_collision(
    PhysicsComponent *physics_component,
    PhysicsComponentSet *physics_component_set
  );
  PhysicsComponent* find_ray_collision(
    Ray *ray,
    PhysicsComponent *physics_component_to_ignore_or_nullptr,
    PhysicsComponentSet *physics_component_set
  );
}

#endif
