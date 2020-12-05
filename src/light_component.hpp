#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H

struct LightComponent : public Component {
  LightType type;
  glm::vec3 direction;
  glm::vec4 color;
  glm::vec4 attenuation;
};

#endif
