#ifndef PHYSICS_HPP
#define PHYSICS_HPP

struct SpatialComponent;

struct Obb {
  glm::vec3 center;
  glm::vec3 axes[2]; // We can get the third axis with a cross product
  glm::vec3 extents;
};

namespace Physics {
  Obb transform_obb(Obb obb, SpatialComponent *spatial_component);
}

#endif
