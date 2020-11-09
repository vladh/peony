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
      SHADER_STANDARD,
      SHADER_DIR"axes.vert", SHADER_DIR"axes.frag", nullptr
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

  // Ocean
  {
    uint32 n_vertices;
    uint32 n_indices;
    real32 *vertex_data;
    uint32 *index_data;
    Util::make_plane(
      &memory->temp_memory_pool,
      200, 200,
      800, 800,
      &n_vertices, &n_indices,
      &vertex_data, &index_data
    );
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      MODELSOURCE_DATA,
      vertex_data, n_vertices,
      index_data, n_indices,
      "ocean",
      GL_TRIANGLES
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "water",
      SHADER_STANDARD,
      SHADER_DIR"water.vert", SHADER_DIR"water.frag",
#if 1
      nullptr
#else
      SHADER_DIR"normal_visualizer.geom"
#endif
    );
    depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "water_depth",
      SHADER_DEPTH,
      SHADER_DIR"water_depth.vert", SHADER_DIR"standard_depth.frag",
      nullptr
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->add(*state->g_position_texture, "g_position_texture");
    texture_set->add(*state->g_albedo_texture, "g_albedo_texture");
    texture_set->add(Texture(TEXTURE_NORMAL, "resources/textures/vlachos.jpg"), "normal_texture");
    texture_set->add(Texture(TEXTURE_OTHER, "resources/textures/water_foam.png"), "foam_texture");
    texture_set->add(
      Texture(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        TEXTURE_SHADOWMAP, state->cube_shadowmaps,
        state->cube_shadowmap_width, state->cube_shadowmap_height, 1
      ),
      "cube_shadowmaps"
    );
    texture_set->add(
      Texture(
        GL_TEXTURE_2D_ARRAY,
        TEXTURE_SHADOWMAP, state->texture_shadowmaps,
        state->texture_shadowmap_width, state->texture_shadowmap_height, 1
      ),
      "texture_shadowmaps"
    );
    *model_asset->mesh_templates.push() = {
      shader_asset, depth_shader_asset, texture_set, true, 0, 0
    };
    memory->temp_memory_pool.reset();
  }

  // Skysphere
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
      "skysphere",
      GL_TRIANGLE_STRIP
    );
    shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory,
      "skysphere",
      SHADER_STANDARD,
      SHADER_DIR"skysphere.vert", SHADER_DIR"skysphere.frag", nullptr
    );
    *model_asset->mesh_templates.push() = {shader_asset, nullptr, nullptr, true, 0, 0};
    memory->temp_memory_pool.reset();
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
      SHADER_STANDARD,
      SHADER_DIR"simple.vert", SHADER_DIR"simple.frag", nullptr
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
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
    );
    texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
    texture_set->add(
      Texture(TEXTURE_ALBEDO, "resources/textures/rocks/2k/Stones_Color.jpg"),
      "albedo_texture"
    );
    texture_set->set_metallic_static(0.0f);
    texture_set->add(
      Texture(TEXTURE_ROUGHNESS, "resources/textures/rocks/2k/Stones_Roughness.jpg"),
      "roughness_texture"
    );
    texture_set->set_ao_static(1.0f);
    texture_set->add(
      Texture(TEXTURE_NORMAL, "resources/textures/rocks/2k/Stones_Normal.jpg"),
      "normal_texture"
    );
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
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
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
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
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

    {
      shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory,
        "entity",
        SHADER_STANDARD,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
      );
      texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
      texture_set->add(
        Texture(TEXTURE_ALBEDO, "resources/textures/shop/03_-_Default_BaseColor.tga.png"),
        "albedo_texture"
      );
      texture_set->add(
        Texture(TEXTURE_METALLIC, "resources/textures/shop/03_-_Default_Metallic.tga.png"),
        "metallic_texture"
      );
      texture_set->add(
        Texture(TEXTURE_ROUGHNESS, "resources/textures/shop/03_-_Default_Roughness.tga.png"),
        "roughness_texture"
      );
      texture_set->add(
        Texture(TEXTURE_AO, "resources/textures/shop/AO-3.tga.png"),
        "ao_texture"
      );
      texture_set->add(
        Texture(TEXTURE_NORMAL, "resources/textures/shop/03_-_Default_Normal.tga.png"),
        "normal_texture"
      );
      *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, false, 0, 0};
    }

    {
      shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory,
        "entity",
        SHADER_STANDARD,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
      );
      texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
      texture_set->add(
        Texture(TEXTURE_ALBEDO, "resources/textures/shop/01_-_Default_BaseColor.tga.png"),
        "albedo_texture"
      );
      texture_set->add(
        Texture(TEXTURE_METALLIC, "resources/textures/shop/01_-_Default_Metallic.tga.png"),
        "metallic_texture"
      );
      texture_set->add(
        Texture(TEXTURE_ROUGHNESS, "resources/textures/shop/01_-_Default_Roughness.tga.png"),
        "roughness_texture"
      );
      texture_set->add(
        Texture(TEXTURE_AO, "resources/textures/shop/AO-1.tga.png"),
        "ao_texture"
      );
      texture_set->add(
        Texture(TEXTURE_NORMAL, "resources/textures/shop/01_-_Default_Normal.tga.png"),
        "normal_texture"
      );
      *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, false, 0, 1};
    }

    {
      shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory,
        "entity",
        SHADER_STANDARD,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
      );
      texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
      texture_set->add(
        Texture(TEXTURE_ALBEDO, "resources/textures/shop/02_-_Default_BaseColor.tga.png"),
        "albedo_texture"
      );
      texture_set->add(
        Texture(TEXTURE_METALLIC, "resources/textures/shop/02_-_Default_Metallic.tga.png"),
        "metallic_texture"
      );
      texture_set->add(
        Texture(TEXTURE_ROUGHNESS, "resources/textures/shop/02_-_Default_Roughness.tga.png"),
        "roughness_texture"
      );
      texture_set->add(
        Texture(TEXTURE_AO, "resources/textures/shop/AO-2.tga.png"),
        "ao_texture"
      );
      texture_set->add(
        Texture(TEXTURE_NORMAL, "resources/textures/shop/02_-_Default_Normal.tga.png"),
        "normal_texture"
      );
      *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, false, 0, 2};
    }
  }

  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;

  // Lighting screenquad
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "lighting",
    SHADER_STANDARD,
    SHADER_DIR"lighting.vert", SHADER_DIR"lighting.frag", nullptr
  );
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad_lighting",
    GL_TRIANGLES
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->g_position_texture, "g_position_texture");
  texture_set->add(*state->g_normal_texture, "g_normal_texture");
  texture_set->add(*state->g_albedo_texture, "g_albedo_texture");
  texture_set->add(*state->g_pbr_texture, "g_pbr_texture");
  texture_set->add(
    Texture(
      GL_TEXTURE_CUBE_MAP_ARRAY,
      TEXTURE_SHADOWMAP, state->cube_shadowmaps,
      state->cube_shadowmap_width, state->cube_shadowmap_height, 1
    ),
    "cube_shadowmaps"
  );
  texture_set->add(
    Texture(
      GL_TEXTURE_2D_ARRAY,
      TEXTURE_SHADOWMAP, state->texture_shadowmaps,
      state->texture_shadowmap_width, state->texture_shadowmap_height, 1
    ),
    "texture_shadowmaps"
  );
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};

  // Preblur screenquad
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "blur",
    SHADER_STANDARD,
    SHADER_DIR"blur.vert", SHADER_DIR"blur.frag", nullptr
  );
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad_preblur",
    GL_TRIANGLES
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->l_bright_color_texture, "source_texture");
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};

  // Blur 1 screenquad
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "blur",
    SHADER_STANDARD,
    SHADER_DIR"blur.vert", SHADER_DIR"blur.frag", nullptr
  );
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad_blur1",
    GL_TRIANGLES
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->blur2_texture, "source_texture");
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};

  // Blur 2 screenquad
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "blur",
    SHADER_STANDARD,
    SHADER_DIR"blur.vert", SHADER_DIR"blur.frag", nullptr
  );
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad_blur2",
    GL_TRIANGLES
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->blur1_texture, "source_texture");
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};

  // Postprocessing screenquad
  shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "postprocessing",
    SHADER_STANDARD,
    SHADER_DIR"postprocessing.vert", SHADER_DIR"postprocessing.frag", nullptr
  );
  model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad_postprocessing",
    GL_TRIANGLES
  );
  texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->l_color_texture, "l_color_texture");
  texture_set->add(*state->l_depth_texture, "l_depth_texture");
  texture_set->add(*state->blur2_texture, "bloom_texture");
  *model_asset->mesh_templates.push() = {shader_asset, nullptr, texture_set, true, 0, 0};
}
