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
    "screenquad", SHADER_DIR"lighting.vert", SHADER_DIR"lighting.frag"
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
  shader_set_mat4(&text_shader_asset->shader, "text_projection", &text_projection);
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
    memory, array_push<ModelAsset>(&state->model_assets),
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad",
    GL_TRIANGLES
  );
  models_set_is_screenquad(&model_asset->model);
  models_add_texture(
    &model_asset->model, TEXTURE_G_POSITION, state->g_position_texture
  );
  models_add_texture(
    &model_asset->model, TEXTURE_G_NORMAL, state->g_normal_texture
  );
  models_add_texture(
    &model_asset->model, TEXTURE_G_ALBEDO, state->g_albedo_texture
  );
  models_add_texture(
    &model_asset->model, TEXTURE_G_PBR, state->g_pbr_texture
  );
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH, state->shadow_cubemaps[idx]
    );
  }

  // Axes
  real32 axes_vertices[] = AXES_VERTICES;
  models_make_asset_from_data(
    memory, array_push<ModelAsset>(&state->model_assets),
    axes_vertices, 6,
    nullptr, 0,
    "axes",
    GL_LINES
  );

  // Alpaca
#if USE_ALPACA
  real32 alpaca_vertices[] = ALPACA_VERTICES;
  model_asset = models_make_asset_from_data(
    memory, array_push<ModelAsset>(&state->model_assets),
    alpaca_vertices, 36,
    nullptr, 0,
    "alpaca",
    GL_TRIANGLES
  );
  models_add_texture(
    &model_asset->model, TEXTURE_DIFFUSE,
    models_load_texture_from_file("resources/textures/alpaca.jpg")
  );
#endif

  // Goose
  model_asset = models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    "goose", "resources/models/", "miniGoose.fbx"
  );
  models_set_static_pbr(
    &model_asset->model, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, 1.0f, 1.0f
  );

  // Sphere
  uint32 n_x_segments = 64;
  uint32 n_y_segments = 64;
  uint32 vertex_data_length = (n_x_segments + 1) * (n_y_segments + 1) * 8 * 2;
  uint32 index_data_length = (n_x_segments + 1) * (n_y_segments + 1) * 2 * 2;

  uint32 n_vertices;
  uint32 n_indices;

  real32 *vertex_data = (real32*)memory_push_memory_to_pool(
    &memory->temp_memory_pool, sizeof(real32) * (vertex_data_length * 2)
  );
  uint32 *index_data = (uint32*)memory_push_memory_to_pool(
    &memory->temp_memory_pool, sizeof(uint32) * (index_data_length * 2)
  );

  util_make_sphere(
    n_x_segments, n_y_segments,
    &n_vertices, &n_indices,
    vertex_data, index_data
  );

  model_asset = models_make_asset_from_data(
    memory, array_push<ModelAsset>(&state->model_assets),
    vertex_data, n_vertices,
    index_data, n_indices,
    "sphere",
    GL_TRIANGLE_STRIP
  );
  memory_reset_pool(&memory->temp_memory_pool);

  models_add_texture(
    &model_asset->model, TEXTURE_ALBEDO,
    models_load_texture_from_file("resources/textures/rusted_iron/albedo.png")
  );
  models_add_texture(
    &model_asset->model, TEXTURE_METALLIC,
    models_load_texture_from_file("resources/textures/rusted_iron/metallic.png")
  );
  models_add_texture(
    &model_asset->model, TEXTURE_ROUGHNESS,
    models_load_texture_from_file("resources/textures/rusted_iron/roughness.png")
  );
  models_add_texture(
    &model_asset->model, TEXTURE_AO,
    models_load_texture_from_file("resources/textures/rusted_iron/ao.png")
  );
  models_add_texture(
    &model_asset->model, TEXTURE_NORMAL,
    models_load_texture_from_file("resources/textures/rusted_iron/normal.png")
  );
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }

  // Floor
  model_asset = models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    "floor", "resources/models/", "cube.obj"
  );
  models_set_static_pbr(
    &model_asset->model, glm::vec4(0.9f, 0.8f, 0.7f, 1.0f), 0.0f, 1.0f, 1.0f
  );
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }

  // Temple
  model_asset = models_make_asset_from_file(
    memory, array_push<ModelAsset>(&state->model_assets),
    /* "temple", "resources/models/", "pantheon.obj" */
    /* "temple", "resources/models/", "Stones_AssetKit.fbx" */
    "temple", "resources/models/", "shop.fbx"
  );

  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_ALBEDO,
    models_load_texture_from_file("resources/textures/shop/03_-_Default_BaseColor.tga.png"),
    0, 0
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_METALLIC,
    models_load_texture_from_file("resources/textures/shop/03_-_Default_Metallic.tga.png"),
    0, 0
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_NORMAL,
    models_load_texture_from_file("resources/textures/shop/03_-_Default_Normal.tga.png"),
    0, 0
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_ROUGHNESS,
    models_load_texture_from_file("resources/textures/shop/03_-_Default_Roughness.tga.png"),
    0, 0
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_AO,
    models_load_texture_from_file("resources/textures/shop/AO-3.tga.png"),
    0, 0
  );

  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_ALBEDO,
    models_load_texture_from_file("resources/textures/shop/01_-_Default_BaseColor.tga.png"),
    0, 1
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_METALLIC,
    models_load_texture_from_file("resources/textures/shop/01_-_Default_Metallic.tga.png"),
    0, 1
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_NORMAL,
    models_load_texture_from_file("resources/textures/shop/01_-_Default_Normal.tga.png"),
    0, 1
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_ROUGHNESS,
    models_load_texture_from_file("resources/textures/shop/01_-_Default_Roughness.tga.png"),
    0, 1
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_AO,
    models_load_texture_from_file("resources/textures/shop/AO-1.tga.png"),
    0, 1
  );

  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_ALBEDO,
    models_load_texture_from_file("resources/textures/shop/02_-_Default_BaseColor.tga.png"),
    0, 2
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_METALLIC,
    models_load_texture_from_file("resources/textures/shop/02_-_Default_Metallic.tga.png"),
    0, 2
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_NORMAL,
    models_load_texture_from_file("resources/textures/shop/02_-_Default_Normal.tga.png"),
    0, 2
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_ROUGHNESS,
    models_load_texture_from_file("resources/textures/shop/02_-_Default_Roughness.tga.png"),
    0, 2
  );
  models_add_texture_for_node_idx(
    &model_asset->model, TEXTURE_AO,
    models_load_texture_from_file("resources/textures/shop/AO-2.tga.png"),
    0, 2
  );

  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    models_add_texture(
      &model_asset->model, TEXTURE_DEPTH,
      state->shadow_cubemaps[idx]
    );
  }
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
