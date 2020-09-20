void scene_resources_init_shaders(Memory *memory, State *state) {
  state->entity_shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    memory,
    "entity",
    SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
  );

  state->entity_depth_shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    memory,
    "entity_depth",
    SHADER_DIR"entity_depth.vert", SHADER_DIR"entity_depth.frag",
    SHADER_DIR"entity_depth.geom"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    memory,
    "axes", SHADER_DIR"axes.vert", SHADER_DIR"axes.frag"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    memory,
    "screenquad", SHADER_DIR"postprocessing.vert", SHADER_DIR"postprocessing.frag"
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    memory,
    "light", SHADER_DIR"light.vert", SHADER_DIR"light.frag"
  );

  ShaderAsset *text_shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    memory,
    "text", SHADER_DIR"text.vert", SHADER_DIR"text.frag"
  );
  glm::mat4 text_projection = glm::ortho(
    0.0f, (real32)state->window_width, 0.0f, (real32)state->window_height
  );
  glUseProgram(text_shader_asset->shader.program);
  shader_set_mat4(text_shader_asset->shader.program, "text_projection", &text_projection);
}

void scene_resources_init_models(Memory *memory, State *state) {
  ModelAsset *model_asset;

  // Light
  models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    "light", "resources/models/", "cube.obj"
  );

  // Screenquad
  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;
  model_asset = models_make_asset_from_data(
    memory,
    array_push<ModelAsset>(&state->model_assets),
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad", GL_TRIANGLES
  );
  models_add_texture(
    &model_asset->model, TEXTURE_DIFFUSE,
    /* state->postprocessing_color_texture */
    state->g_position_texture
  );
  models_add_texture(
    &model_asset->model, TEXTURE_DIFFUSE,
    state->g_normal_texture
  );
  models_add_texture(
    &model_asset->model, TEXTURE_DIFFUSE,
    state->g_albedospec_texture
  );
#if USE_SHADOWS
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
#endif

  // Axes
  real32 axes_vertices[] = AXES_VERTICES;
  models_make_asset_from_data(
    memory,
    array_push<ModelAsset>(&state->model_assets),
    axes_vertices, 6,
    nullptr, 0,
    "axes", GL_LINES
  );

  // Alpaca
  real32 alpaca_vertices[] = ALPACA_VERTICES;
  model_asset = models_make_asset_from_data(
    memory,
    array_push<ModelAsset>(&state->model_assets),
    alpaca_vertices, 36,
    nullptr, 0,
    "alpaca", GL_TRIANGLES
  );
  models_add_texture(
    &model_asset->model, TEXTURE_DIFFUSE,
    models_load_texture_from_file("resources/textures/alpaca.jpg")
  );

  // Goose
  model_asset = models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    "goose", "resources/models/", "miniGoose.fbx"
  );
#if USE_SHADOWS
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
#endif

  // Floor
  model_asset = models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    "floor", "resources/models/", "cube.obj"
  );
#if USE_SHADOWS
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
#endif

  // Temple
  model_asset = models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    "temple", "resources/models/", "pantheon.obj"
  );
#if USE_SHADOWS
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
#endif
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
    "main-font",
    /* "resources/fonts/AlrightSans-Regular.otf" */
    "resources/fonts/iosevka-regular.ttf"
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
