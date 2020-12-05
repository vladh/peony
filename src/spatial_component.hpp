#ifndef SPATIAL_COMPONENT_H
#define SPATIAL_COMPONENT_H

class SpatialComponent : public Component {
public:
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
  SpatialComponent *parent;

  glm::mat4 make_model_matrix();
};

#endif
