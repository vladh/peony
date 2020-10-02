void scene_resources_init_shaders(Memory *memory, State *state) {
  state->entity_depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "entity_depth",
    SHADER_ENTITY_DEPTH,
    SHADER_DIR"entity_depth.vert", SHADER_DIR"entity_depth.frag",
    SHADER_DIR"entity_depth.geom"
  );

  ShaderAsset *text_shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "text",
    SHADER_UI,
    SHADER_DIR"text.vert", SHADER_DIR"text.frag"
  );
  glm::mat4 text_projection = glm::ortho(
    0.0f, (real32)state->window_width, 0.0f, (real32)state->window_height
  );
  glUseProgram(text_shader_asset->program);
  text_shader_asset->set_mat4("text_projection", &text_projection);
  state->text_shader_asset = text_shader_asset;
}

void scene_resources_init_models(Memory *memory, State *state) {
  ModelAsset *model_asset;
  TextureSet *texture_set;
  ShaderAsset *shader_asset;

  // ===============================
  // Models loaded from data
  // ===============================

  // Axes
  real32 axes_vertices[] = AXES_VERTICES;
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "axes",
    SHADER_OTHER_OBJECT,
    SHADER_DIR"axes.vert", SHADER_DIR"axes.frag"
  );
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    axes_vertices, 6,
    nullptr, 0,
    "axes",
    GL_LINES
  );
  model_asset->bind_shader_and_texture(shader_asset, nullptr);

  // Sphere
  uint32 n_x_segments = 64;
  uint32 n_y_segments = 64;
  uint32 vertex_data_length = (n_x_segments + 1) * (n_y_segments + 1) * 8 * 2;
  uint32 index_data_length = (n_x_segments + 1) * (n_y_segments + 1) * 2 * 2;

  uint32 n_vertices;
  uint32 n_indices;

  real32 *vertex_data = (real32*)memory->temp_memory_pool.push(
    sizeof(real32) * (vertex_data_length * 2), "sphere_vertex_data"
  );
  uint32 *index_data = (uint32*)memory->temp_memory_pool.push(
    sizeof(uint32) * (index_data_length * 2), "sphere_index_data"
  );

  Util::make_sphere(
    n_x_segments, n_y_segments,
    &n_vertices, &n_indices,
    vertex_data, index_data
  );

  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    vertex_data, n_vertices,
    index_data, n_indices,
    "sphere",
    GL_TRIANGLE_STRIP
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "entity",
    SHADER_ENTITY,
    SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
  );
  texture_set = model_asset->create_texture_set();
  texture_set->albedo_texture = ResourceManager::load_texture_from_file(
    "resources/textures/rusted_iron/albedo.png"
  );
  texture_set->metallic_texture = ResourceManager::load_texture_from_file(
    "resources/textures/rusted_iron/metallic.png"
  );
  texture_set->roughness_texture = ResourceManager::load_texture_from_file(
    "resources/textures/rusted_iron/roughness.png"
  );
  texture_set->ao_texture = ResourceManager::load_texture_from_file(
    "resources/textures/rusted_iron/ao.png"
  );
  texture_set->normal_texture = ResourceManager::load_texture_from_file(
    "resources/textures/rusted_iron/normal.png"
  );
  model_asset->bind_shader_and_texture(shader_asset, texture_set);

  // ===============================
  // Models loaded from files
  // ===============================

  // Light
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "light", "resources/models/", "cube.obj"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "light",
    SHADER_OTHER_OBJECT,
    SHADER_DIR"light.vert", SHADER_DIR"light.frag"
  );
  model_asset->bind_shader_and_texture(shader_asset, nullptr);

  // Cart
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "cart", "resources/models/", "cart.obj"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "cart",
    SHADER_ENTITY,
    SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
  );
  texture_set = model_asset->create_texture_set();
  texture_set->albedo_static = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);
  texture_set->metallic_static = 0.0f;
  texture_set->roughness_static = 0.8f;
  texture_set->ao_static = 1.0f;
  model_asset->bind_shader_and_texture(shader_asset, texture_set);


  // Goose
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "goose", "resources/models/", "miniGoose.fbx"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "entity",
    SHADER_ENTITY,
    SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
  );
  texture_set = model_asset->create_texture_set();
  texture_set->albedo_static = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  texture_set->metallic_static = 0.0f;
  texture_set->roughness_static = 1.0f;
  texture_set->ao_static = 1.0f;
  model_asset->bind_shader_and_texture(shader_asset, texture_set);

  // Floor
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "floor", "resources/models/", "cube.obj"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "entity",
    SHADER_ENTITY,
    SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
  );
  texture_set = model_asset->create_texture_set();
  texture_set->albedo_static = glm::vec4(0.9f, 0.8f, 0.7f, 1.0f);
  texture_set->metallic_static = 0.0f;
  texture_set->roughness_static = 1.0f;
  texture_set->ao_static = 1.0f;
  model_asset->bind_shader_and_texture(shader_asset, texture_set);

  // Temple
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "temple", "resources/models/", "shop.fbx"
  );

  {
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_ENTITY,
      SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
    );
    texture_set = model_asset->create_texture_set();
    texture_set->albedo_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/03_-_Default_BaseColor.tga.png"
    );
    texture_set->metallic_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/03_-_Default_Metallic.tga.png"
    );
    texture_set->normal_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/03_-_Default_Normal.tga.png"
    );
    texture_set->roughness_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/03_-_Default_Roughness.tga.png"
    );
    texture_set->ao_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/AO-3.tga.png"
    );
    model_asset->bind_shader_and_texture_for_node_idx(shader_asset, texture_set, 0, 0);
  }

  {
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_ENTITY,
      SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
    );
    texture_set = model_asset->create_texture_set();
    texture_set->albedo_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/01_-_Default_BaseColor.tga.png"
    );
    texture_set->metallic_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/01_-_Default_Metallic.tga.png"
    );
    texture_set->normal_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/01_-_Default_Normal.tga.png"
    );
    texture_set->roughness_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/01_-_Default_Roughness.tga.png"
    );
    texture_set->ao_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/AO-1.tga.png"
    );
    model_asset->bind_shader_and_texture_for_node_idx(shader_asset, texture_set, 0, 1);
  }

  {
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_ENTITY,
      SHADER_DIR"entity.vert", SHADER_DIR"entity.frag"
    );
    texture_set = model_asset->create_texture_set();
    texture_set->albedo_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/02_-_Default_BaseColor.tga.png"
    );
    texture_set->metallic_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/02_-_Default_Metallic.tga.png"
    );
    texture_set->normal_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/02_-_Default_Normal.tga.png"
    );
    texture_set->roughness_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/02_-_Default_Roughness.tga.png"
    );
    texture_set->ao_texture = ResourceManager::load_texture_from_file(
      "resources/textures/shop/AO-2.tga.png"
    );
    model_asset->bind_shader_and_texture_for_node_idx(shader_asset, texture_set, 0, 2);
  }
}

void scene_resources_init_fonts(Memory *memory, State *state) {
  FT_Library ft_library;

  if (FT_Init_FreeType(&ft_library)) {
    log_error("Could not init FreeType");
    return;
  }

  new(state->font_assets.push()) FontAsset(
    memory,
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
