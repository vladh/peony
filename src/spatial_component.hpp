#ifndef SPATIAL_COMPONENT_H
#define SPATIAL_COMPONENT_H

struct SpatialComponent : public Component {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
  SpatialComponent *parent;
};

#endif
