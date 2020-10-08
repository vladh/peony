void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Lights
  entity = state->entity_manager.add("light1");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec4(-7.0f, 3.0f, 0.0f, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RENDERPASS_FORWARD
  );
  state->light_component_manager.add(
    entity->handle,
    glm::vec4(9.0f, 9.0f, 9.0f, 1.0f),
    glm::vec4(1.0f, 0.09f, 0.032f, 0.0f)
  );
  state->lights.push(entity->handle);

  entity = state->entity_manager.add("light2");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec4(-4.0f, 16.0f, 0.0f, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RENDERPASS_FORWARD
  );
  state->light_component_manager.add(
    entity->handle,
    glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
  );
  state->lights.push(entity->handle);

  // Temple
#if 1
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
#if 1
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

  // Cart
#if 0
  entity = state->entity_manager.add("cart");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(-5.1f, 1.45f, 2.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.003f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "cart"),
    RENDERPASS_DEFERRED
  );
#endif

  // Spheres
#if 1
  uint32 n_spheres = 1;
  for (uint16 idx = 0; idx < n_spheres; idx++) {
    entity = state->entity_manager.add("sphere");
    state->spatial_component_manager.add(
      entity->handle,
      glm::vec3(-5.0f, 0.65f, -3.0f),
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
#if 0
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
    RENDERPASS_FORWARD
  );
#endif
}
