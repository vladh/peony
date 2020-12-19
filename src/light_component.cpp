uint32 light_type_to_int(LightType light_type) {
  if (light_type == LightType::point) {
    return 1;
  } else if (light_type == LightType::directional) {
    return 2;
  }
  return 0;
}


bool32 LightComponent::is_valid() {
  return this->type != LightType::none;
}


LightComponent::LightComponent() {};


LightComponent::LightComponent(
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
