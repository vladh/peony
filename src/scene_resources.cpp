void scene_resources_init_shaders(Memory *memory, State *state) {
  state->entity_shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "entity",
    "src/shaders/entity.vert", "src/shaders/entity.frag"
  );

  state->entity_depth_shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "entity_depth",
    "src/shaders/entity_depth.vert", "src/shaders/entity_depth.frag",
    "src/shaders/entity_depth.geom"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "axes", "src/shaders/axes.vert", "src/shaders/axes.frag"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets), "screenquad",
    "src/shaders/postprocessing.vert", "src/shaders/postprocessing.frag"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "light", "src/shaders/light.vert", "src/shaders/light.frag"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "text", "src/shaders/text.vert", "src/shaders/text.frag"
  );
}

void scene_resources_init_models(Memory *memory, State *state) {
  // Goose
  ModelAsset *goose_model_asset = models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    "goose", "resources/models/", "miniGoose.fbx"
  );
#if USE_SHADOWS
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture_to_mesh(
      &goose_model_asset->model.meshes.items[0],
      TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
#endif

  // Axes
  real32 axes_vertices[] = AXES_VERTICES;
  models_make_asset_from_data(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    axes_vertices, 6,
    nullptr, 0,
    "axes", GL_LINES
  );

  // Alpaca
  real32 alpaca_vertices[] = ALPACA_VERTICES;
  ModelAsset *alpaca_model_asset = models_make_asset_from_data(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    alpaca_vertices, 36,
    nullptr, 0,
    "alpaca", GL_TRIANGLES
  );
  models_add_texture_to_mesh(
    &alpaca_model_asset->model.meshes.items[0],
    TEXTURE_DIFFUSE,
    models_load_texture_from_file("resources/textures/alpaca.jpg")
  );

  // Screenquad
  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;
  ModelAsset *screenquad_model_asset = models_make_asset_from_data(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad", GL_TRIANGLES
  );
  models_add_texture_to_mesh(
    &screenquad_model_asset->model.meshes.items[0],
    TEXTURE_DIFFUSE,
    state->postprocessing_color_texture
  );

  // Floor
  ModelAsset *floor_model_asset = models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    "floor", "resources/models/", "cube.obj"
  );
#if USE_SHADOWS
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture_to_mesh(
      &floor_model_asset->model.meshes.items[0],
      TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
#endif

  // Light
  models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    "light", "resources/models/", "cube.obj"
  );
}

void scene_resources_init_fonts(Memory *memory, State *state) {
  FT_Library ft_library;

  if (FT_Init_FreeType(&ft_library)) {
    log_error("Could not init FreeType");
    return;
  }

  font_make_asset(
    memory,
    array_push<FontAsset>(&state->font_assets),
    &ft_library,
    "alright-sans-regular",
    "resources/fonts/AlrightSans-Regular.otf"
  );

  FT_Done_FreeType(ft_library);

  glGenVertexArrays(1, &state->text_vao);
  glGenBuffers(1, &state->text_vbo);
  glBindVertexArray(state->text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
