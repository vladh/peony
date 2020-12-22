#ifndef LIGHT_COMPONENT_HPP
#define LIGHT_COMPONENT_HPP

enum class LightType {none, point, directional};

const char* light_type_to_string(LightType light_type);
LightType light_type_from_string(const char *str);
uint32 light_type_to_int(LightType light_type);

class LightComponent {
public:
  EntityHandle entity_handle = Entity::no_entity_handle;
  LightType type = LightType::none;
  glm::vec3 direction = glm::vec3(0.0f);
  glm::vec4 color = glm::vec4(0.0f);
  glm::vec4 attenuation = glm::vec4(0.0f);

  bool32 is_valid();
  LightComponent();
  LightComponent(
    EntityHandle entity_handle,
    LightType type,
    glm::vec3 direction,
    glm::vec4 color,
    glm::vec4 attenuation
  );
};

#endif
