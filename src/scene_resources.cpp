void scene_resources_init(Memory *memory, State *state) {
  ModelAsset *model_asset;
  TextureSetAsset *texture_set;
  ShaderAsset *shader_asset;

  state->standard_depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "standard_depth",
    SHADER_STANDARD_DEPTH,
    SHADER_DIR"standard_depth.vert", SHADER_DIR"standard_depth.frag",
    SHADER_DIR"standard_depth.geom"
  );

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
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, true, 0, 0};

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
    SHADER_STANDARD,
    SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
    "resources/textures/rusted_iron/albedo.png",
    "resources/textures/rusted_iron/metallic.png",
    "resources/textures/rusted_iron/roughness.png",
    "resources/textures/rusted_iron/ao.png",
    "resources/textures/rusted_iron/normal.png"
  );
  *model_asset->mesh_templates.push() = {shader_asset, texture_set, true, 0, 0};

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
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, true, 0, 0};

  // Cart
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "cart", "resources/models/", "cart.obj"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "cart",
    SHADER_STANDARD,
    SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
    glm::vec4(0.2f, 0.2f, 1.0f, 1.0f), 0.0f, 0.8f, 1.0f
  );
  *model_asset->mesh_templates.push() = {shader_asset, texture_set, true, 0, 0};

  // Goose
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "goose", "resources/models/", "miniGoose.fbx"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "entity",
    SHADER_STANDARD,
    SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, 1.0f, 1.0f
  );
  *model_asset->mesh_templates.push() = {shader_asset, texture_set, true, 0, 0};

  // Floor
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "floor", "resources/models/", "cube.obj"
  );
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "entity",
    SHADER_STANDARD,
    SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
    glm::vec4(0.9f, 0.8f, 0.7f, 1.0f), 0.0f, 1.0f, 1.0f
  );
  *model_asset->mesh_templates.push() = {shader_asset, texture_set, true, 0, 0};

  // Temple
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory, MODELSOURCE_FILE, "temple", "resources/models/", "shop.fbx"
  );

  {
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_STANDARD,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
      "resources/textures/shop/03_-_Default_BaseColor.tga.png",
      "resources/textures/shop/03_-_Default_Metallic.tga.png",
      "resources/textures/shop/03_-_Default_Roughness.tga.png",
      "resources/textures/shop/AO-3.tga.png",
      "resources/textures/shop/03_-_Default_Normal.tga.png"
    );
    *model_asset->mesh_templates.push() = {shader_asset, texture_set, false, 0, 0};
  }

  {
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_STANDARD,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
      "resources/textures/shop/01_-_Default_BaseColor.tga.png",
      "resources/textures/shop/01_-_Default_Metallic.tga.png",
      "resources/textures/shop/01_-_Default_Roughness.tga.png",
      "resources/textures/shop/AO-1.tga.png",
      "resources/textures/shop/01_-_Default_Normal.tga.png"
    );
    *model_asset->mesh_templates.push() = {shader_asset, texture_set, false, 0, 1};
  }

  {
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_STANDARD,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSetAsset(
      "resources/textures/shop/02_-_Default_BaseColor.tga.png",
      "resources/textures/shop/02_-_Default_Metallic.tga.png",
      "resources/textures/shop/02_-_Default_Roughness.tga.png",
      "resources/textures/shop/AO-2.tga.png",
      "resources/textures/shop/02_-_Default_Normal.tga.png"
    );
    *model_asset->mesh_templates.push() = {shader_asset, texture_set, false, 0, 2};
  }
}
