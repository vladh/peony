#ifndef PHYSICS_HPP
#define PHYSICS_HPP

struct Obb {
  glm::vec3 center;
  glm::vec3 axes[2]; // We can get the third axis with a cross product
  glm::vec3 extents;
};

namespace Physics {
  bool32 is_obb_valid(Obb *obb);
  Obb apply_model_matrix_to_obb(Obb obb, glm::mat4 *model_matrix);
}

#endif
