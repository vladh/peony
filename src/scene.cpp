void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Lights
#if 1
  glm::vec3 light_direction = glm::vec3(1.0f, -0.5f, 0.0f);
  entity = state->entity_manager.add("directional_light");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RENDERPASS_FORWARD_NODEPTH
  );
  state->light_component_manager.add(
    entity->handle,
    LIGHT_DIRECTIONAL,
    light_direction,
    glm::vec4(4.0f, 4.0f, 4.0f, 1.0f),
    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
  );
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec4(-light_direction * DIRECTIONAL_LIGHT_DISTANCE, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->lights.push(entity->handle);
#endif

#if 0
  entity = state->entity_manager.add("point_light");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec4(-7.0f, 3.0f, 0.0f, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RENDERPASS_FORWARD_NODEPTH
  );
  state->light_component_manager.add(
    entity->handle,
    LIGHT_POINT,
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec4(5.0f, 5.0f, 5.0f, 1.0f),
    glm::vec4(1.0f, 0.09f, 0.032f, 0.0f)
  );
  state->lights.push(entity->handle);
#endif

  // Plane
#if 1
  entity = state->entity_manager.add("plane");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(1.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "plane"),
    RENDERPASS_FORWARD_DEPTH
  );
#endif

  // Temple
#if 0
  entity = state->entity_manager.add("temple");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    glm::vec3(0.1f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "temple"),
    RENDERPASS_DEFERRED
  );
#endif

  // Geese
#if 0
  entity = state->entity_manager.add("goose");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(-4.6f, 0.00f, -1.5f),
    glm::angleAxis(glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    glm::vec3(0.2f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "goose"),
    RENDERPASS_DEFERRED
  );
  state->geese.push(entity->handle);
#endif

  // Floor
#if 0
  entity = state->entity_manager.add("floor");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(150.0f, 0.1f, 150.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "floor"),
    RENDERPASS_DEFERRED
  );
#endif

  // Rocks
#if 1
  entity = state->entity_manager.add("rocks");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, -2.0f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.03f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "rocks"),
    RENDERPASS_DEFERRED
  );
#endif

  // Spheres
#if 0
  uint32 n_spheres = 8;
  for (uint16 idx = 0; idx < n_spheres; idx++) {
    entity = state->entity_manager.add("sphere");
    state->spatial_component_manager.add(
      entity->handle,
      glm::vec3(Util::random(-3.0f, 3.0f), 0.0f, Util::random(-3.0f, 3.0f)),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.5f)
    );
    state->drawable_component_manager.add(
      entity->handle,
      ModelAsset::get_by_name(&state->model_assets, "sphere"),
      RENDERPASS_DEFERRED
    );
    state->spheres.push(entity->handle);
  }
#endif

  // Axes
#if 1
  entity = state->entity_manager.add("axes");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(1.0f, 1.0f, 1.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "axes"),
    RENDERPASS_FORWARD_NODEPTH
  );
#endif

  // Screenquad
  entity = state->entity_manager.add("screenquad");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad"),
    RENDERPASS_LIGHTING
  );
}
