void scene_init_lights(Memory *memory, State *state) {
  Light *light1 = array_push(&state->lights);
  light1->position = glm::vec3(0.0f, 1.0f, 0.0f);
  light1->direction = glm::vec3(0.0f, 0.0f, 0.0f);
  light1->color = glm::vec4(1.0f, 0.8f, 0.6f, 1.0f);
  /* light1->color = glm::vec4(300.0f, 300.0f, 300.0f, 1.0f); */
  light1->ambient = glm::vec3(0.5f, 0.5f, 0.5f);
  light1->diffuse = glm::vec3(0.5f, 0.5f, 0.5f) * glm::vec3(light1->color);
  light1->specular = glm::vec3(1.0f, 1.0f, 1.0f);
  light1->attenuation_constant = 1.0f;
  light1->attenuation_linear = 0.09f;
  light1->attenuation_quadratic = 0.032f;

  Light *light2 = array_push(&state->lights);
  light2->position = glm::vec3(0.0f, 1.0f, 0.0f);
  light2->direction = glm::vec3(0.0f, 0.0f, 0.0f);
  light2->color = glm::vec4(1.0f, 0.8f, 0.6f, 1.0f);
  /* light2->color = glm::vec4(300.0f, 300.0f, 300.0f, 1.0f); */
  light2->ambient = glm::vec3(0.0f, 0.0f, 0.0f);
  light2->diffuse = glm::vec3(20.0f, 20.0f, 20.0f) * glm::vec3(light2->color);
  light2->specular = glm::vec3(1.0f, 1.0f, 1.0f);
  light2->attenuation_constant = 1.0f;
  light2->attenuation_linear = 0.09f;
  light2->attenuation_quadratic = 0.032f;
}

void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Axes
  entity = entity_make(
    array_push<Entity>(&state->entities),
    "axes",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::vec3(1.0f),
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
    glm::vec3(25.0f, 0.0f, 0.0f),
    glm::vec3(0.7f),
    glm::angleAxis(
      glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)
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

  uint32 n_geese = 10;
  for (uint8 idx = 0; idx < n_geese; idx++) {
    real32 scale = (real32)util_random(0.2f, 0.4f);
    entity = entity_make(
      array_push<Entity>(&state->entities),
      "goose",
      ENTITY_MODEL,
      glm::vec3(0.0f),
      glm::vec3(scale),
      glm::angleAxis(
        glm::radians(-90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );
    entity_set_model_asset(entity, asset_get_model_asset_by_name(&state->model_assets, "goose"));
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
      glm::vec3(scale, scale, scale),
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
  state->lights.items[0].position = glm::vec3(
    sin(state->t) * 3.0f,
    1.0f,
    0.0f
  );
  state->lights.items[1].position = glm::vec3(
    cos(state->t) * 3.0f,
    3.0f,
    sin(state->t) * 5.0f
  );

  // Light entities
  entity_get_all_with_tag(
    state->entities, "light", &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    entity->position = state->lights.items[idx].position;
  }

  // Geese
  entity_get_all_with_name(
    state->entities, "goose", &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];

    real32 x_offset = -10.0f;
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
      glm::vec3(0.0f, 0.0f, 1.0f)
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
