const char* Entities::behavior_to_string(Behavior behavior) {
  if (behavior == Behavior::none) {
    return "none";
  } else if (behavior == Behavior::test) {
    return "test";
  } else if (behavior == Behavior::char_movement_test) {
    return "char_movement_test";
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
  } else if (strcmp(str, "char_movement_test") == 0) {
    return Behavior::char_movement_test;
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


bool32 Entities::is_animation_component_valid(
  AnimationComponent *animation_component
) {
  return animation_component->n_bones > 0 &&
    animation_component->n_animations > 0;
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


bool32 Entities::is_model_loader_valid(ModelLoader *model_loader) {
  return model_loader->state != ModelLoaderState::empty;
}


bool32 Entities::is_entity_loader_valid(EntityLoader *entity_loader) {
  return entity_loader->state != EntityLoaderState::empty;
}
