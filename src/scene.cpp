void scene_init_lights(Memory *memory, State *state) {
  Light *light1 = array_push(&state->lights);
  light1->position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  light1->direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  light1->color = glm::vec4(9.0f, 9.0f, 9.0f, 1.0f);
  light1->attenuation = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);

  Light *light2 = array_push(&state->lights);
  light2->position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  light2->direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  /* light2->color = glm::vec4(12.0f, 10.0f, 8.0f, 1.0f); */
  light2->color = glm::vec4(7.0f, 6.0f, 5.0f, 1.0f);
  light2->attenuation = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Axes
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "axes",
    ENTITY_MODEL,
    RENDERPASS_FORWARD,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "axes"));

  // Floor
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "floor",
    ENTITY_MODEL,
    RENDERPASS_DEFERRED,
    glm::vec3(0.0f),
    glm::vec3(150.0f, 0.1f, 150.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "floor"));

  // Temple
  for (uint32 idx = 0; idx < 2; idx++) {
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "temple",
      ENTITY_MODEL,
      RENDERPASS_DEFERRED,
      glm::vec3(0.0f, 0.1f, 5.0f * idx),
      glm::vec3(0.1f),
      glm::angleAxis(
        glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
      )
    );
    entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "temple"));
  }

  // Light entities
  for (uint32 idx = 0; idx < state->lights.size; idx++) {
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "light",
      ENTITY_MODEL,
      RENDERPASS_FORWARD,
      state->lights.items[idx].position,
      glm::vec3(0.3f, 0.3f, 0.3f),
      glm::angleAxis(
        glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );
    entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "light"));
    entity_add_tag(entity, "light");
  }

  // Geese
  uint32 n_geese = 10;
  for (uint8 idx = 0; idx < n_geese; idx++) {
    real32 scale = (real32)util_random(0.2f, 0.4f);
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "goose",
      ENTITY_MODEL,
      RENDERPASS_DEFERRED,
      glm::vec3(-20.0f, 0.0f, (real32)idx * 2.0f),
      glm::vec3(scale),
      glm::angleAxis(
        glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );
    entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "goose"));
  }

  // Spheres
  uint32 n_spheres = 10;
  for (uint8 idx = 0; idx < n_spheres; idx++) {
    real32 scale = 0.8f;
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "sphere",
      ENTITY_MODEL,
      RENDERPASS_DEFERRED,
      glm::vec3(-10.0f, 1.0f, (real32)idx * 2.0f),
      glm::vec3(scale),
      glm::angleAxis(
        glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );
    entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "sphere"));
  }

  // Screenquad
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "screenquad",
    ENTITY_SCREENQUAD,
    RENDERPASS_LIGHTING,
    glm::vec3(0.0f),
    glm::vec3(1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f))
  );
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "screenquad"));
}

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

  for (uint32 idx = 0; idx < state->entities.size; idx++) {
    Entity *entity = &state->entities.items[idx];
    update_entity(state, entity);
  }
}
