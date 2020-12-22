const char* light_type_to_string(LightType light_type) {
  if (light_type == LightType::none) {
    return "none";
  } else if (light_type == LightType::point) {
    return "point";
  } else if (light_type == LightType::directional) {
    return "directional";
  } else {
    log_error("Don't know how to convert LightType to string: %d", light_type);
    return "<unknown>";
  }
}


LightType light_type_from_string(const char *str) {
  if (strcmp(str, "none") == 0) {
    return LightType::none;
  } else if (strcmp(str, "point") == 0) {
    return LightType::point;
  } else if (strcmp(str, "directional") == 0) {
    return LightType::directional;
  } else {
    log_fatal("Could not parse LightType: %s", str);
    return LightType::none;
  }
}


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
