const char* entities::light_type_to_string(LightType light_type) {
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


LightType entities::light_type_from_string(const char *str) {
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


uint32 entities::light_type_to_int(LightType light_type) {
  if (light_type == LightType::point) {
    return 1;
  } else if (light_type == LightType::directional) {
    return 2;
  }
  return 0;
}


bool32 entities::is_light_component_valid(
  LightComponent *light_component
) {
  return light_component->type != LightType::none;
}


const char* entities::behavior_to_string(Behavior behavior) {
  if (behavior == Behavior::none) {
    return "none";
  } else if (behavior == Behavior::test) {
    return "test";
  } else if (behavior == Behavior::char_movement_test) {
    return "char_movement_test";
  } else {
    logs::error("Don't know how to convert Behavior to string: %d", behavior);
    return "<unknown>";
  }
}


Behavior entities::behavior_from_string(const char *str) {
  if (strcmp(str, "none") == 0) {
    return Behavior::none;
  } else if (strcmp(str, "test") == 0) {
    return Behavior::test;
  } else if (strcmp(str, "char_movement_test") == 0) {
    return Behavior::char_movement_test;
  } else {
    logs::fatal("Could not parse Behavior: %s", str);
    return Behavior::none;
  }
}


bool32 entities::is_behavior_component_valid(
  BehaviorComponent *behavior_component
) {
  return behavior_component->behavior != Behavior::none;
}


bool32 entities::is_animation_component_valid(
  AnimationComponent *animation_component
) {
  return animation_component->n_bones > 0 &&
    animation_component->n_animations > 0;
}


bool32 entities::is_physics_component_valid(
  PhysicsComponent *physics_component
) {
  return physics_component->obb.extents.x > 0;
}


void entities::print_spatial_component(
  SpatialComponent *spatial_component
) {
  logs::info("SpatialComponent:");
  logs::info("  entity_handle: %d", spatial_component->entity_handle);
  logs::info("  position:");
  logs::print_v3(&spatial_component->position);
  logs::info("  rotation:");
  logs::info("(don't know how to print rotation, sorry)");
  /* logs::print_v4(&spatial_component->rotation); */
  logs::info("  scale:");
  logs::print_v3(&spatial_component->scale);
  logs::info("  parent_entity_handle: %d", spatial_component->parent_entity_handle);
}


bool32 entities::does_spatial_component_have_dimensions(
  SpatialComponent *spatial_component
) {
  return (
    spatial_component->scale.x > 0.0f &&
    spatial_component->scale.y > 0.0f &&
    spatial_component->scale.z > 0.0f
  );
}


bool32 entities::is_spatial_component_valid(
  SpatialComponent *spatial_component
) {
  return does_spatial_component_have_dimensions(spatial_component) ||
    spatial_component->parent_entity_handle != Entity::no_entity_handle;
}


bool32 entities::is_model_loader_valid(ModelLoader *model_loader) {
  return model_loader->state != ModelLoaderState::empty;
}


bool32 entities::is_entity_loader_valid(EntityLoader *entity_loader) {
  return entity_loader->state != EntityLoaderState::empty;
}
