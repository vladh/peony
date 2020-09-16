void init_geese(Memory *memory, State *state) {
  uint32 n_geese = 10;

  for (uint8 idx = 0; idx < n_geese; idx++) {
    real64 scale = util_random(0.2f, 0.4f);
    Entity *entity = entity_make(
      array_push<Entity>(&state->entities),
      "goose",
      ENTITY_MODEL,
      glm::vec3(
        util_random(-8.0f, 8.0f),
        0.1f,
        util_random(-8.0f, 8.0f)
      ),
      glm::vec3(scale, scale, scale),
      glm::angleAxis(
        glm::radians(-90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );

    entity_set_color(entity, glm::vec3(1.0f, 0.0f, 0.0f));
    entity_set_model_asset(
      entity, asset_get_model_asset_by_name(&state->model_assets, "goose")
    );
    entity_add_tag(entity, "goose");
  }
}

void init_axes(Memory *memory, State *state) {
  Entity *entity = entity_make(
    array_push<Entity>(&state->entities),
    "axes",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );

  entity_set_shader_asset(
    entity, asset_get_shader_asset_by_name(&state->shader_assets, "axes")
  );
  entity_set_model_asset(
    entity, asset_get_model_asset_by_name(&state->model_assets, "axes")
  );
  entity_add_tag(entity, "axes");
}

void init_alpaca(Memory *memory, State *state) {
  uint32 n_alpacas = 10;

  for (uint8 idx = 0; idx < n_alpacas; idx++) {
    real64 scale = util_random(1.0f, 1.4f);
    Entity *entity = entity_make(
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

    entity_set_model_asset(
      entity, asset_get_model_asset_by_name(&state->model_assets, "alpaca")
    );
    entity_add_tag(entity, "alpaca");
  }
}

void init_screenquad(Memory *memory, State *state) {
  Entity *entity = entity_make(
    array_push<Entity>(&state->entities),
    "screenquad",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f))
  );

  entity_set_shader_asset(
    entity, asset_get_shader_asset_by_name(&state->shader_assets, "screenquad")
  );
  entity_set_model_asset(
    entity, asset_get_model_asset_by_name(&state->model_assets, "screenquad")
  );
  entity_add_tag(entity, "screenquad");
}

void init_floor(Memory *memory, State *state) {
  Entity *entity = entity_make(
    array_push<Entity>(&state->entities),
    "floor",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(150.0f, 0.1f, 150.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );

  entity_set_color(entity, glm::vec3(1.0f, 1.0f, 1.0f));
  entity_set_model_asset(
    entity, asset_get_model_asset_by_name(&state->model_assets, "floor")
  );
  entity_add_tag(entity, "floor");
}

void init_lights(Memory *memory, State *state) {
  Entity *light1 = entity_make(
    array_push<Entity>(&state->entities),
    "light1",
    ENTITY_MODEL,
    state->lights.items[0].position,
    glm::vec3(0.3f, 0.3f, 0.3f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_shader_asset(
    light1, asset_get_shader_asset_by_name(&state->shader_assets, "light")
  );
  entity_set_model_asset(
    light1, asset_get_model_asset_by_name(&state->model_assets, "light")
  );
  entity_add_tag(light1, "light");

  Entity *light2 = entity_make(
    array_push<Entity>(&state->entities),
    "light2",
    ENTITY_MODEL,
    state->lights.items[1].position,
    glm::vec3(0.3f, 0.3f, 0.3f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_shader_asset(
    light2, asset_get_shader_asset_by_name(&state->shader_assets, "light")
  );
  entity_set_model_asset(
    light2, asset_get_model_asset_by_name(&state->model_assets, "light")
  );
  entity_add_tag(light2, "light");
}

void scene_init_objects(Memory *memory, State *state) {
  init_axes(memory, state);
  init_floor(memory, state);
  init_lights(memory, state);
  init_geese(memory, state);
#if USE_POSTPROCESSING || USE_SHADOWS
  init_screenquad(memory, state);
#endif
#if USE_ALPACA
  init_alpaca(memory, state);
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

  // Shadow transforms
  camera_create_shadow_transforms(
    state->shadow_transforms, state->lights.items[0].position,
    state->shadow_map_width, state->shadow_map_height,
    state->shadow_near_clip_dist, state->shadow_far_clip_dist
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

    real32 period_offset = (real32)idx;
    real32 spin_speed_factor = 0.3f;
    real32 radius_offset = (2.0f + (idx * 1.0f));
    real32 pos_arg = ((real32)state->t * spin_speed_factor) + period_offset;
    real32 spin_deg_per_t = 90.0f;

    entity->position = glm::vec3(
      sin(pos_arg) * radius_offset,
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
