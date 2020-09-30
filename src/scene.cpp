void scene_init_screenquad(Memory *memory, State *state) {
  ModelAsset *model_asset;
  TextureSet *texture_set;
  Entity *entity;

  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;
  model_asset = models_make_asset_from_data(
    memory, array_push<ModelAsset>(&state->model_assets),
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad",
    GL_TRIANGLES
  );
  texture_set = models_add_texture_set(&model_asset->model);
  texture_set->g_position_texture = state->g_position_texture;
  texture_set->g_normal_texture = state->g_normal_texture;
  texture_set->g_albedo_texture = state->g_albedo_texture;
  texture_set->g_pbr_texture = state->g_pbr_texture;
  texture_set->n_depth_textures = state->n_shadow_framebuffers;
  log_info("Initialising %d depth textures", texture_set->n_depth_textures);
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    texture_set->depth_textures[idx] = state->shadow_cubemaps[idx];
  }
  models_set_texture_set(&model_asset->model, texture_set);
  models_set_shader_asset(
    &model_asset->model,
    shader_make_asset(
      array_push<ShaderAsset>(&state->shader_assets),
      memory,
      "lighting",
      SHADER_LIGHTING,
      SHADER_DIR"lighting.vert", SHADER_DIR"lighting.frag"
    )
  );

  entity = state->entity_manager.add("screenquad");
  state->drawable_component_manager.add(
    entity->handle,
    asset_get_model_asset_by_name(&state->model_assets, "screenquad"),
    RENDERPASS_LIGHTING
  );
}

void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Axes
  entity = state->entity_manager.add("axes");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(1.0f, 1.0f, 1.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    asset_get_model_asset_by_name(&state->model_assets, "axes"),
    RENDERPASS_FORWARD
  );

  // Floor
  entity = state->entity_manager.add("floor");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(150.0f, 0.1f, 150.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    asset_get_model_asset_by_name(&state->model_assets, "floor"),
    RENDERPASS_DEFERRED
  );

  // Temple
  for (uint32 idx = 0; idx < 1; idx++) {
    entity = state->entity_manager.add("temple");
    state->spatial_component_manager.add(
      entity->handle,
      glm::vec3(0.0f, 0.1f, 0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      glm::vec3(0.1f)
    );
    state->drawable_component_manager.add(
      entity->handle,
      asset_get_model_asset_by_name(&state->model_assets, "temple"),
      RENDERPASS_DEFERRED
    );
  }

  // Lights
  entity = state->entity_manager.add("light1");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec4(-7.0f, 2.0f, 0.0f, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    asset_get_model_asset_by_name(&state->model_assets, "light"),
    RENDERPASS_FORWARD
  );
  state->light_component_manager.add(
    entity->handle,
    glm::vec4(9.0f, 9.0f, 9.0f, 1.0f),
    glm::vec4(1.0f, 0.09f, 0.032f, 0.0f)
  );
  array_push<EntityHandle>(&state->lights, entity->handle);

  entity = state->entity_manager.add("light2");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec4(-4.0f, 16.0f, 0.0f, 1.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    asset_get_model_asset_by_name(&state->model_assets, "light"),
    RENDERPASS_FORWARD
  );
  state->light_component_manager.add(
    entity->handle,
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
  );
  array_push<EntityHandle>(&state->lights, entity->handle);

  // Geese
  uint32 n_geese = 10;
  for (uint8 idx = 0; idx < n_geese; idx++) {
    entity = state->entity_manager.add("goose");
    state->spatial_component_manager.add(
      entity->handle,
      glm::vec3(-20.0f, 0.0f, -5.0f + (real32)idx * 2.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.3f)
    );
    state->drawable_component_manager.add(
      entity->handle,
      asset_get_model_asset_by_name(&state->model_assets, "goose"),
      RENDERPASS_DEFERRED
    );
    array_push<EntityHandle>(&state->geese, entity->handle);
  }

  // Spheres
  uint32 n_spheres = 10;
  for (uint8 idx = 0; idx < n_spheres; idx++) {
    entity = state->entity_manager.add("sphere");
    state->spatial_component_manager.add(
      entity->handle,
      glm::vec3(-8.0f, 1.0f, -5.0f + (real32)idx * 2.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.8f)
    );
    state->drawable_component_manager.add(
      entity->handle,
      asset_get_model_asset_by_name(&state->model_assets, "sphere"),
      RENDERPASS_DEFERRED
    );
    array_push<EntityHandle>(&state->spheres, entity->handle);
  }
}


void scene_update(Memory *memory, State *state) {
  // TODO: Think of a better way to do this.
  // We should rather be iterating through all SpatialComponents rather
  // than looking everything up.

  // Lights
  {
    SpatialComponent *spatial = state->spatial_component_manager.get(
      state->lights.items[0]
    );

    real64 time_term = (sin(state->t / 1.5f) + 1.0f) / 2.0f * (PI / 2.0f) + (PI / 2.0f);
    real64 x_term = 0.0f + cos(time_term) * 8.0f;
    real64 z_term = 0.0f + sin(time_term) * 8.0f;

    spatial->position.x = (real32)x_term;
    spatial->position.z = (real32)z_term;
  }

  // Geese
  {
    real32 spin_deg_per_t = 90.0f;
    for (uint32 idx = 0; idx < state->geese.size; idx++) {
      SpatialComponent *spatial = state->spatial_component_manager.get(
        state->geese.items[idx]
      );
      spatial->rotation *= glm::angleAxis(
        glm::radians(spin_deg_per_t * (real32)state->dt),
        glm::vec3(0.0f, 1.0f, 0.0f)
      );
    }
  }

  // Spheres
  {
    real32 spin_deg_per_t = 45.0f;
    for (uint32 idx = 0; idx < state->spheres.size; idx++) {
      SpatialComponent *spatial = state->spatial_component_manager.get(
        state->spheres.items[idx]
      );
      spatial->rotation *= glm::angleAxis(
        glm::radians(spin_deg_per_t * (real32)state->dt),
        glm::vec3(0.0f, 1.0f, 0.0f)
      );
    }
  }
}
