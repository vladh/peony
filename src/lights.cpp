const char* lights::light_type_to_string(LightType light_type) {
  if (light_type == LightType::none) {
    return "none";
  } else if (light_type == LightType::point) {
    return "point";
  } else if (light_type == LightType::directional) {
    return "directional";
  } else {
    logs::error("Don't know how to convert LightType to string: %d", light_type);
    return "<unknown>";
  }
}


LightType lights::light_type_from_string(const char *str) {
  if (strcmp(str, "none") == 0) {
    return LightType::none;
  } else if (strcmp(str, "point") == 0) {
    return LightType::point;
  } else if (strcmp(str, "directional") == 0) {
    return LightType::directional;
  } else {
    logs::fatal("Could not parse LightType: %s", str);
    return LightType::none;
  }
}


uint32 lights::light_type_to_int(LightType light_type) {
  if (light_type == LightType::point) {
    return 1;
  } else if (light_type == LightType::directional) {
    return 2;
  }
  return 0;
}


bool32 lights::is_light_component_valid(LightComponent *light_component) {
  return light_component->type != LightType::none;
}


void lights::update_light_components(
  LightComponentSet *light_component_set,
  SpatialComponentSet *spatial_component_set,
  real64 t,
  v3 camera_position,
  real32 dir_light_angle
) {
  for_each (light_component, light_component_set->components) {
    if (light_component->entity_handle == Entity::no_entity_handle) {
      continue;
    }

    SpatialComponent *spatial_component =
      spatial_component_set->components[light_component->entity_handle];

    if (!(
      is_light_component_valid(light_component) &&
      spatial::is_spatial_component_valid(spatial_component)
    )) {
      continue;
    }

    if (light_component->type == LightType::point) {
      light_component->color.b = ((real32)sin(t) + 1.0f) / 2.0f * 50.0f;
    }

    // For the sun! :)
    if (light_component->type == LightType::directional) {
      spatial_component->position = camera_position +
        -light_component->direction * DIRECTIONAL_LIGHT_DISTANCE;
      light_component->direction = v3(
        sin(dir_light_angle), -cos(dir_light_angle), 0.0f
      );
    }
  }
}
