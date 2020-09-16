void scene_resources_init_shaders(Memory *memory, State *state) {
  shader_make_asset(
    &state->entity_shader_asset,
    "entity",
    "src/shaders/entity.vert", "src/shaders/entity.frag"
  );

  shader_make_asset(
    &state->entity_depth_shader_asset,
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
#if USE_POSTPROCESSING
    "src/shaders/postprocessing.vert", "src/shaders/postprocessing.frag"
#elif USE_SHADOWS
    "src/shaders/depth.vert", "src/shaders/depth.frag"
#else
    "src/shaders/postprocessing.vert", "src/shaders/postprocessing.frag"
#endif
  );

  shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "light", "src/shaders/light.vert", "src/shaders/light.frag"
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
    "goose", "resources/", "miniGoose.fbx"
  );
#if USE_SHADOWS
  models_add_texture_to_mesh(
    &goose_model_asset->model.meshes.items[0],
    TEXTURE_DEPTH,
    state->shadow_cubemap
  );
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
    models_load_texture_from_file("resources/alpaca.jpg")
  );

  // Screenquad
  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;
  models_make_asset_from_data(
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

  // Floor
  ModelAsset *floor_model_asset = models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    "floor", "resources/", "cube.obj"
  );
#if USE_SHADOWS
  models_add_texture_to_mesh(
    &floor_model_asset->model.meshes.items[0],
    TEXTURE_DEPTH,
    state->shadow_cubemap
  );
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
    "light", "resources/", "cube.obj"
  );
}