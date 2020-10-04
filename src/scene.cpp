void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

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

  // Temple
#if 1
  uint32 n_temples = 1;
  for (uint32 idx = 0; idx < n_temples; idx++) {
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
  }
#endif

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

  // Geese
#if 0
  uint32 n_geese = 1;
  for (uint16 idx = 0; idx < n_geese; idx++) {
    entity = state->entity_manager.add("goose");
    state->spatial_component_manager.add(
      entity->handle,
      glm::vec3(-20.0f, 0.0f, -5.0f + (real32)idx * 2.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.3f)
    );
    state->drawable_component_manager.add(
      entity->handle,
      ModelAsset::get_by_name(&state->model_assets, "goose"),
      RENDERPASS_DEFERRED
    );
    state->geese.push(entity->handle);
  }
#endif

  // Spheres
#if 0
  uint32 n_spheres = 1;
  for (uint16 idx = 0; idx < n_spheres; idx++) {
    entity = state->entity_manager.add("sphere");
    state->spatial_component_manager.add(
      entity->handle,
      /* glm::vec3(-8.0f, 1.0f, -5.0f + (real32)idx * 2.0f), */
      glm::vec3(Util::random(-20.0f, 20.0f), 1.0f, Util::random(-20.0f, 20.0f)),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.8f)
    );
    state->drawable_component_manager.add(
      entity->handle,
      ModelAsset::get_by_name(&state->model_assets, "sphere"),
      RENDERPASS_DEFERRED
    );
    state->spheres.push(entity->handle);
  }
#endif
}
