#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H

enum class LightType {none, point, directional};

uint32 light_type_to_int(LightType light_type) {
  if (light_type == LightType::point) {
    return 1;
  } else if (light_type == LightType::directional) {
    return 2;
  }
  return 0;
}

struct LightComponent {
  EntityHandle entity_handle = Entity::no_entity_handle;
  LightType type = LightType::none;
  glm::vec3 direction = glm::vec3(0.0f);
  glm::vec4 color = glm::vec4(0.0f);
  glm::vec4 attenuation = glm::vec4(0.0f);

  bool32 is_valid() {
    return this->type != LightType::none;
  }

  LightComponent() {};

  LightComponent(
    EntityHandle entity_handle,
    LightType type,
    glm::vec3 direction,
    glm::vec4 color,
    glm::vec4 attenuation
  ) :
    entity_handle(entity_handle),
    type(type),
    direction(direction),
    color(color),
    attenuation(attenuation)
  {
  }
};

#endif
