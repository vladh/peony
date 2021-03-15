const char* Entities::behavior_to_string(Behavior behavior) {
  if (behavior == Behavior::none) {
    return "none";
  } else if (behavior == Behavior::test) {
    return "test";
  } else {
    log_error("Don't know how to convert Behavior to string: %d", behavior);
    return "<unknown>";
  }
}


Behavior Entities::behavior_from_string(const char *str) {
  if (strcmp(str, "none") == 0) {
    return Behavior::none;
  } else if (strcmp(str, "test") == 0) {
    return Behavior::test;
  } else {
    log_fatal("Could not parse Behavior: %s", str);
    return Behavior::none;
  }
}


bool32 Entities::is_behavior_component_valid(
  BehaviorComponent *behavior_component
) {
  return behavior_component->behavior != Behavior::none;
}


BehaviorComponent* Entities::init_behavior_component(
  BehaviorComponent *behavior_component,
  EntityHandle entity_handle,
  Behavior behavior
) {
  behavior_component->entity_handle = entity_handle;
  behavior_component->behavior = behavior;
  return behavior_component;
}


const char* Entities::light_type_to_string(LightType light_type) {
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


LightType Entities::light_type_from_string(const char *str) {
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


uint32 Entities::light_type_to_int(LightType light_type) {
  if (light_type == LightType::point) {
    return 1;
  } else if (light_type == LightType::directional) {
    return 2;
  }
  return 0;
}


bool32 Entities::is_light_component_valid(
  LightComponent *light_component
) {
  return light_component->type != LightType::none;
}


LightComponent* Entities::init_light_component(
  LightComponent *light_component,
  EntityHandle entity_handle,
  LightType type,
  glm::vec3 direction,
  glm::vec4 color,
  glm::vec4 attenuation
) {
  light_component->entity_handle = entity_handle;
  light_component->type = type;
  light_component->direction = direction;
  light_component->color = color;
  light_component->attenuation = attenuation;
  return light_component;
}


void Entities::print_spatial_component(
  SpatialComponent *spatial_component
) {
  log_info("SpatialComponent:");
  log_info("  entity_handle: %d", spatial_component->entity_handle);
  log_info("  position:");
  log_vec3(&spatial_component->position);
  log_info("  rotation:");
  log_info("(don't know how to print rotation, sorry)");
  /* log_vec4(&spatial_component->rotation); */
  log_info("  scale:");
  log_vec3(&spatial_component->scale);
  log_info("  parent_entity_handle: %d", spatial_component->parent_entity_handle);
}


bool32 Entities::does_spatial_component_have_dimensions(
  SpatialComponent *spatial_component
) {
  return (
    spatial_component->scale.x > 0.0f &&
    spatial_component->scale.y > 0.0f &&
    spatial_component->scale.z > 0.0f
  );
}


bool32 Entities::is_spatial_component_valid(
  SpatialComponent *spatial_component
) {
  return does_spatial_component_have_dimensions(spatial_component) ||
    spatial_component->parent_entity_handle != Entity::no_entity_handle;
}


SpatialComponent* Entities::init_spatial_component(
  SpatialComponent *spatial_component,
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale,
  EntityHandle parent_entity_handle
) {
  spatial_component->entity_handle = entity_handle;
  spatial_component->position = position;
  spatial_component->rotation = rotation;
  spatial_component->scale = scale;
  spatial_component->parent_entity_handle = parent_entity_handle;
  return spatial_component;
}


SpatialComponent* Entities::init_spatial_component(
  SpatialComponent *spatial_component,
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) {
  spatial_component->entity_handle = entity_handle;
  spatial_component->position = position;
  spatial_component->rotation = rotation;
  spatial_component->scale = scale;
  return spatial_component;
}


bool32 Entities::is_drawable_component_valid(
  DrawableComponent *drawable_component
) {
  return drawable_component->mesh != nullptr;
}


DrawableComponent* Entities::init_drawable_component(
  DrawableComponent *drawable_component,
  EntityHandle entity_handle,
  Mesh *mesh,
  RenderPassFlag target_render_pass
) {
  if (!mesh) {
    log_fatal("Invalid mesh when creating DrawableComponent.");
  }
  drawable_component->entity_handle = entity_handle;
  drawable_component->mesh = mesh;
  drawable_component->target_render_pass = target_render_pass;
  return drawable_component;
}
