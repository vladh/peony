#ifndef PHYSICS_HPP
#define PHYSICS_HPP

struct SpatialComponent;
struct PhysicsComponent;
struct PhysicsComponentSet;

struct Obb {
  glm::vec3 center;
  glm::vec3 x_axis; // We can get the z axis with a cross product
  glm::vec3 y_axis; // We can get the z axis with a cross product
  glm::vec3 extents;
};

struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;
};

namespace Physics {
  Obb transform_obb(Obb obb, SpatialComponent *spatial_component);
  bool32 intersect_obb_obb(Obb *obb1, Obb *obb2);
  PhysicsComponent* find_collision(
    PhysicsComponent *physics_component,
    PhysicsComponentSet *physics_component_set
  );
}

#endif
