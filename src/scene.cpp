void scene_init_lights(Memory *memory, State *state) {
  Light *light1 = array_push(&state->lights);
  light1->position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  light1->direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  light1->color = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
  light1->attenuation = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);

  Light *light2 = array_push(&state->lights);
  light2->position = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  light2->direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  light2->color = glm::vec4(14.0f, 12.0f, 10.0f, 1.0f);
  light2->attenuation = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Axes
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "axes",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_shader_asset(entity, asset_get_shader_asset_by_name(&state->shader_assets, "axes"));
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "axes"));

  // Floor
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "floor",
    ENTITY_MODEL,
    glm::vec3(0.0f),
    glm::vec3(150.0f, 0.1f, 150.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "floor"));

  // Temple
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "temple",
    ENTITY_MODEL,
    glm::vec3(0.0f),
    glm::vec3(0.1f),
    /* glm::vec3(25.0f, 0.0f, 0.0f), */
    /* glm::vec3(0.7f, 0.7f, 0.7f), */
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
    )
  );
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "temple"));

  // Light entities
  for (uint32 idx = 0; idx < state->lights.size; idx++) {
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "light",
      ENTITY_MODEL,
      state->lights.items[idx].position,
      glm::vec3(0.3f, 0.3f, 0.3f),
      glm::angleAxis(
        glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );
    entity_set_shader_asset(entity, asset_get_shader_asset_by_name(&state->shader_assets, "light"));
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
      glm::vec3(0.0f, 0.0f, 0.0f),
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
      glm::vec3(0.0f, 1.0f, 0.0f),
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
    ENTITY_MODEL,
    glm::vec3(0.0f),
    glm::vec3(1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f))
  );
  entity_set_shader_asset(entity, asset_get_shader_asset_by_name(&state->shader_assets, "screenquad"));
  entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "screenquad"));

#if USE_ALPACA
  // Alpaca
  uint32 n_alpacas = 10;
  for (uint8 idx = 0; idx < n_alpacas; idx++) {
    real64 scale = util_random(1.0f, 1.4f);
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "alpaca",
      ENTITY_MODEL,
      glm::vec3(
        util_random(-6.0f, 6.0f),
        util_random(1.0f, 6.0f),
        util_random(-6.0f, 6.0f)
      ),
      glm::vec3(scale),
      glm::angleAxis(
        glm::radians(-90.0f + (30.0f * idx)), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );

    entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "alpaca"));
  }
#endif
}

void scene_update(Memory *memory, State *state) {
  // Lights
  state->lights.items[0].position = glm::vec4(
    sin(state->t) * 3.0f - 6.0f,
    2.0f,
    0.0f,
    state->lights.items[0].position.w
  );
  state->lights.items[1].position = glm::vec4(
    -4.0f,
    16.0f,
    0.0f,
    state->lights.items[1].position.w
  );

  // Light entities
  entity_get_all_with_tag(
    state->entities, "light", &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    entity->position = glm::vec3(state->lights.items[idx].position);
  }

  // Spheres
  entity_get_all_with_name(
    state->entities, "sphere", &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];

    real32 spin_deg_per_t = 90.0f;

    entity->position = glm::vec3(
      -10.0f, entity->position.y, (real32)idx * 2.0f
    );
    entity->rotation *= glm::angleAxis(
      glm::radians(spin_deg_per_t * (real32)state->dt),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );
  }

  // Geese
  entity_get_all_with_name(
    state->entities, "goose", &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];

    real32 x_offset = -30.0f;
    real32 period_offset = (real32)idx;
    /* real32 spin_speed_factor = 0.3f; */
    real32 spin_speed_factor = 0.0f;
    real32 radius_offset = (2.0f + (idx * 0.5f));
    real32 pos_arg = ((real32)state->t * spin_speed_factor) + period_offset;
    real32 spin_deg_per_t = 90.0f;

    entity->position = glm::vec3(
      x_offset + sin(pos_arg) * radius_offset,
      entity->position.y,
      cos(pos_arg) * radius_offset
    );
    entity->rotation *= glm::angleAxis(
      glm::radians(spin_deg_per_t * (real32)state->dt),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );
  }

  // Alpaca
  entity_get_all_with_name(
    state->entities, "alpaca", &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    entity->rotation *= glm::angleAxis(
      glm::radians(15.0f * (real32)state->dt),
      glm::vec3(1.0f, 0.0f, 0.0f)
    );
  }
}
