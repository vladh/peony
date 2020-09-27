void scene_init_lights(Memory *memory, State *state) {
  Light *light1 = array_push(&state->lights);
  light1->position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  light1->color = glm::vec4(9.0f, 9.0f, 9.0f, 1.0f);
  light1->attenuation = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);

  Light *light2 = array_push(&state->lights);
  light2->position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  light2->color = glm::vec4(7.0f, 6.0f, 5.0f, 1.0f);
  light2->attenuation = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Axes
  entity = state->entity_manager.add("axes");
  state->entity_manager.add_spatial_component(
    entity,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(1.0f, 1.0f, 1.0f)
  );
  state->entity_manager.add_drawable_component(
    entity,
    asset_get_model_asset_by_name(&state->model_assets, "axes"),
    RENDERPASS_FORWARD
  );

  // Floor
  entity = state->entity_manager.add("floor");
  state->entity_manager.add_spatial_component(
    entity,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(150.0f, 0.1f, 150.0f)
  );
  state->entity_manager.add_drawable_component(
    entity,
    asset_get_model_asset_by_name(&state->model_assets, "floor"),
    RENDERPASS_DEFERRED
  );

  // Temple
  for (uint32 idx = 0; idx < 2; idx++) {
    entity = state->entity_manager.add("temple");
    state->entity_manager.add_spatial_component(
      entity,
      glm::vec3(0.0f, 0.1f, 5.0f * idx),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      glm::vec3(0.1f)
    );
    state->entity_manager.add_drawable_component(
      entity,
      asset_get_model_asset_by_name(&state->model_assets, "temple"),
      RENDERPASS_DEFERRED
    );
  }

  // Light entities
  for (uint32 idx = 0; idx < state->lights.size; idx++) {
    entity = state->entity_manager.add("light");
    state->entity_manager.add_spatial_component(
      entity,
      state->lights.items[idx].position,
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.3f)
    );
    state->entity_manager.add_drawable_component(
      entity,
      asset_get_model_asset_by_name(&state->model_assets, "light"),
      RENDERPASS_FORWARD
    );
  }

  // Geese
  uint32 n_geese = 10;
  for (uint8 idx = 0; idx < n_geese; idx++) {
    entity = state->entity_manager.add("goose");
    state->entity_manager.add_spatial_component(
      entity,
      glm::vec3(-20.0f, 0.0f, (real32)idx * 2.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.3f)
    );
    state->entity_manager.add_drawable_component(
      entity,
      asset_get_model_asset_by_name(&state->model_assets, "goose"),
      RENDERPASS_DEFERRED
    );
  }

  // Spheres
  uint32 n_spheres = 10;
  for (uint8 idx = 0; idx < n_spheres; idx++) {
    entity = state->entity_manager.add("sphere");
    state->entity_manager.add_spatial_component(
      entity,
      glm::vec3(-10.0f, 1.0f, (real32)idx * 2.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.8f)
    );
    state->entity_manager.add_drawable_component(
      entity,
      asset_get_model_asset_by_name(&state->model_assets, "sphere"),
      RENDERPASS_DEFERRED
    );
  }

  // Screenquad
  entity = state->entity_manager.add("screenquad");
  state->entity_manager.add_drawable_component(
    entity,
    asset_get_model_asset_by_name(&state->model_assets, "screenquad"),
    RENDERPASS_LIGHTING
  );

  state->entity_manager.print_all();
}

#if 0
void update_entity(State *state, Entity *entity) {
  if (strcmp(entity->name, "light") == 0) {
  } else if (strcmp(entity->name, "sphere") == 0) {
    real32 spin_deg_per_t = 45.0f;
    entity->rotation *= glm::angleAxis(
      glm::radians(spin_deg_per_t * (real32)state->dt),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );
  } else if (strcmp(entity->name, "goose") == 0) {
    real32 spin_deg_per_t = 90.0f;
    entity->rotation *= glm::angleAxis(
      glm::radians(spin_deg_per_t * (real32)state->dt),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );
  } else if (strcmp(entity->name, "temple") == 0) {
    if (entity->position.z != 0) {
      entity->position.z = (real32)sin(state->t) * 5;
    }
  }
}
#endif

void scene_update(Memory *memory, State *state) {
  // Lights
  state->lights.items[0].position = glm::vec4(
    -5.0f,
    2.0f,
    sin(state->t) * 3.0f + 5.0f,
    state->lights.items[0].position.w
  );
  state->lights.items[1].position = glm::vec4(
    -4.0f,
    16.0f,
    0.0f,
    state->lights.items[1].position.w
  );

#if 0
  for (uint32 idx = 0; idx < state->entities.size; idx++) {
    Entity *entity = &state->entities.items[idx];
    update_entity(state, entity);
  }
#endif
}
