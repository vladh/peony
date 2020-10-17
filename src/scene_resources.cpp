void scene_init_resources(Memory *memory, State *state) {
  ModelAsset *model_asset;
  TextureSet *texture_set;
  ShaderAsset *shader_asset;
  ShaderAsset *depth_shader_asset;

  state->standard_depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "standard_depth",
    SHADER_DEPTH,
    SHADER_DIR"standard_depth.vert", SHADER_DIR"standard_depth.frag",
    SHADER_DIR"standard_depth.geom"
  );

  // ===============================
  // Models loaded from data
  // ===============================

  // Axes
  {
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
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, nullptr, true, 0, 0};
  }

  // Plane
  {
    uint32 n_vertices;
    uint32 n_indices;
    real32 *vertex_data;
    uint32 *index_data;
    Util::make_plane(
      &memory->temp_memory_pool,
      100, 100,
      512, 512,
      &n_vertices, &n_indices,
      &vertex_data, &index_data
    );
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      MODELSOURCE_DATA,
      vertex_data, n_vertices,
      index_data, n_indices,
      "plane",
      GL_TRIANGLES
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "water",
      SHADER_STANDARD,
      SHADER_DIR"water.vert", SHADER_DIR"water.frag"
      /* SHADER_DIR"water.vert", SHADER_DIR"water.frag", */
      /* SHADER_DIR"normal_visualizer.geom" */
    );
    depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "water_depth",
      SHADER_DEPTH,
      SHADER_DIR"water_depth.vert", SHADER_DIR"standard_depth.frag",
      SHADER_DIR"standard_depth.geom"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->set_albedo_static(glm::vec4(0.0f, 0.0f, 0.9f, 1.0f));
    texture_set->set_metallic_static(0.0f);
    texture_set->set_roughness_static(1.0f);
    texture_set->set_ao_static(1.0f);
    texture_set->add(*state->g_albedo_texture);
    texture_set->add(Texture(
        GL_TEXTURE_CUBE_MAP_ARRAY, TEXTURE_SHADOWMAP, "shadowmap", state->shadowmap,
        state->shadowmap_width, state->shadowmap_height, 1
    ));
    *model_asset->mesh_templates.push() = {
      shader_asset, depth_shader_asset, texture_set, true, 0, 0
    };
  }

  // Sphere
  {
    uint32 n_vertices;
    uint32 n_indices;
    real32 *vertex_data;
    uint32 *index_data;
    Util::make_sphere(
      &memory->temp_memory_pool,
      64, 64,
      &n_vertices, &n_indices,
      &vertex_data, &index_data
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
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->add(Texture(TEXTURE_ALBEDO, "albedo_texture", "resources/textures/rusted_iron/albedo.png"));
    texture_set->add(Texture(TEXTURE_METALLIC, "metallic_texture", "resources/textures/rusted_iron/metallic.png"));
    texture_set->add(Texture(TEXTURE_ROUGHNESS, "roughness_texture", "resources/textures/rusted_iron/roughness.png"));
    texture_set->set_ao_static(1.0f);
    texture_set->add(Texture(TEXTURE_NORMAL, "normal_texture", "resources/textures/rusted_iron/normal.png"));
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};
  }

  // ===============================
  // Models loaded from files
  // ===============================

  // Light
  {
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory, MODELSOURCE_FILE, "light", "resources/models/", "cube.obj"
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "light",
      SHADER_OTHER_OBJECT,
      SHADER_DIR"light.vert", SHADER_DIR"light.frag"
    );
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, nullptr, true, 0, 0};
  }

  // Rocks
  {
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory, MODELSOURCE_FILE, "rocks", "resources/models/", "Stones_AssetKit.fbx"
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "stones",
      SHADER_STANDARD,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->add(Texture(TEXTURE_ALBEDO, "albedo_texture", "resources/textures/rocks/2k/Stones_Color.jpg"));
    texture_set->set_metallic_static(0.0f);
    texture_set->add(Texture(TEXTURE_ROUGHNESS, "roughness_texture", "resources/textures/rocks/2k/Stones_Roughness.jpg"));
    texture_set->set_ao_static(1.0f);
    texture_set->add(Texture(TEXTURE_NORMAL, "normal_texture", "resources/textures/rocks/2k/Stones_Normal.jpg"));
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};
  }

  // Goose
  {
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory, MODELSOURCE_FILE, "goose", "resources/models/", "miniGoose.fbx"
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_STANDARD,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->set_albedo_static(glm::vec4(0.6f, 0.1f, 0.1f, 1.0f));
    texture_set->set_metallic_static(0.0f);
    texture_set->set_roughness_static(1.0f);
    texture_set->set_ao_static(1.0f);
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};
  }

  // Floor
  {
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory, MODELSOURCE_FILE, "floor", "resources/models/", "cube.obj"
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "entity",
      SHADER_STANDARD,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->set_albedo_static(glm::vec4(0.9f, 0.8f, 0.7f, 1.0f));
    texture_set->set_metallic_static(0.0f);
    texture_set->set_roughness_static(1.0f);
    texture_set->set_ao_static(1.0f);
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};
  }

  // Temple
  {
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory, MODELSOURCE_FILE, "temple", "resources/models/", "shop.fbx"
    );

#if 1
    {
      shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory,
        "entity",
        SHADER_STANDARD,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
      );
      texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
      texture_set->add(Texture(TEXTURE_ALBEDO, "albedo_texture", "resources/textures/shop/03_-_Default_BaseColor.tga.png"));
      texture_set->add(Texture(TEXTURE_METALLIC, "metallic_texture", "resources/textures/shop/03_-_Default_Metallic.tga.png"));
      texture_set->add(Texture(TEXTURE_ROUGHNESS, "roughness_texture", "resources/textures/shop/03_-_Default_Roughness.tga.png"));
      texture_set->add(Texture(TEXTURE_AO, "ao_texture", "resources/textures/shop/AO-3.tga.png"));
      texture_set->add(Texture(TEXTURE_NORMAL, "normal_texture", "resources/textures/shop/03_-_Default_Normal.tga.png"));
      *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, false, 0, 0};
    }

    {
      shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory,
        "entity",
        SHADER_STANDARD,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
      );
      texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
      texture_set->add(Texture(TEXTURE_ALBEDO, "albedo_texture", "resources/textures/shop/01_-_Default_BaseColor.tga.png"));
      texture_set->add(Texture(TEXTURE_METALLIC, "metallic_texture", "resources/textures/shop/01_-_Default_Metallic.tga.png"));
      texture_set->add(Texture(TEXTURE_ROUGHNESS, "roughness_texture","resources/textures/shop/01_-_Default_Roughness.tga.png"));
      texture_set->add(Texture(TEXTURE_AO, "ao_texture", "resources/textures/shop/AO-1.tga.png"));
      texture_set->add(Texture(TEXTURE_NORMAL, "normal_texture", "resources/textures/shop/01_-_Default_Normal.tga.png"));
      *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, false, 0, 1};
    }

    {
      shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory,
        "entity",
        SHADER_STANDARD,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag"
      );
      texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
      texture_set->add(Texture(TEXTURE_ALBEDO, "albedo_texture", "resources/textures/shop/02_-_Default_BaseColor.tga.png"));
      texture_set->add(Texture(TEXTURE_METALLIC, "metallic_texture", "resources/textures/shop/02_-_Default_Metallic.tga.png"));
      texture_set->add(Texture(TEXTURE_ROUGHNESS, "roughness_texture", "resources/textures/shop/02_-_Default_Roughness.tga.png"));
      texture_set->add(Texture(TEXTURE_AO, "ao_texture", "resources/textures/shop/AO-2.tga.png"));
      texture_set->add(Texture(TEXTURE_NORMAL, "normal_texture", "resources/textures/shop/02_-_Default_Normal.tga.png"));
      *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, false, 0, 2};
    }
#endif
  }

  // Screenquad
  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "lighting",
    SHADER_LIGHTING,
    SHADER_DIR"lighting.vert", SHADER_DIR"lighting.frag"
  );

  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad",
    GL_TRIANGLES
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->g_position_texture);
  texture_set->add(*state->g_normal_texture);
  texture_set->add(*state->g_albedo_texture);
  texture_set->add(*state->g_pbr_texture);
  texture_set->add(Texture(
      GL_TEXTURE_CUBE_MAP_ARRAY, TEXTURE_SHADOWMAP, "shadowmap", state->shadowmap,
      state->shadowmap_width, state->shadowmap_height, 1
  ));
  *model_asset->mesh_templates.push() = {
    shader_asset, nullptr, texture_set, true, 0, 0
  };
}
