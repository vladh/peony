void scene_init(Memory *memory, State *state) {
  ModelAsset *model_asset;
  Material *material;

  state->standard_depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory, "standard_depth", ShaderType::depth,
    SHADER_DIR"standard_depth.vert", SHADER_DIR"standard_depth.frag",
    SHADER_DIR"standard_depth.geom"
  );

  // Internal
  {
    real32 screenquad_vertices[] = SCREENQUAD_VERTICES;

    // Lighting screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      screenquad_vertices, 6,
      nullptr, 0,
      "screenquad_lighting",
      GL_TRIANGLES,
      RenderPass::lighting,
      state->entity_manager.add("screenquad_lighting")
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "lighting", ShaderType::standard,
      SHADER_DIR"lighting.vert", SHADER_DIR"lighting.frag", nullptr
    );
    material->add(*state->g_position_texture, "g_position_texture");
    material->add(*state->g_normal_texture, "g_normal_texture");
    material->add(*state->g_albedo_texture, "g_albedo_texture");
    material->add(*state->g_pbr_texture, "g_pbr_texture");
    material->add(
      Texture(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        TextureType::shadowmap, state->cube_shadowmaps,
        state->cube_shadowmap_width, state->cube_shadowmap_height, 1
      ),
      "cube_shadowmaps"
    );
    material->add(
      Texture(
        GL_TEXTURE_2D_ARRAY,
        TextureType::shadowmap, state->texture_shadowmaps,
        state->texture_shadowmap_width, state->texture_shadowmap_height, 1
      ),
      "texture_shadowmaps"
    );

    // Preblur screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      screenquad_vertices, 6,
      nullptr, 0,
      "screenquad_preblur",
      GL_TRIANGLES,
      RenderPass::preblur,
      state->entity_manager.add("screenquad_preblur")
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "blur", ShaderType::standard,
      SHADER_DIR"blur.vert", SHADER_DIR"blur.frag", nullptr
    );
    material->add(*state->l_bright_color_texture, "source_texture");

    // Blur 1 screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      screenquad_vertices, 6,
      nullptr, 0,
      "screenquad_blur1",
      GL_TRIANGLES,
      RenderPass::blur1,
      state->entity_manager.add("screenquad_blur1")
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "blur", ShaderType::standard,
      SHADER_DIR"blur.vert", SHADER_DIR"blur.frag", nullptr
    );
    material->add(*state->blur2_texture, "source_texture");

    // Blur 2 screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      screenquad_vertices, 6,
      nullptr, 0,
      "screenquad_blur2",
      GL_TRIANGLES,
      RenderPass::blur2,
      state->entity_manager.add("screenquad_blur2")
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "blur", ShaderType::standard,
      SHADER_DIR"blur.vert", SHADER_DIR"blur.frag", nullptr
    );
    material->add(*state->blur1_texture, "source_texture");

    // Postprocessing screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      screenquad_vertices, 6,
      nullptr, 0,
      "screenquad_postprocessing",
      GL_TRIANGLES,
      RenderPass::postprocessing,
      state->entity_manager.add("screenquad_postprocessing")
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "postprocessing", ShaderType::standard,
      SHADER_DIR"postprocessing.vert", SHADER_DIR"postprocessing.frag", nullptr
    );
    material->add(*state->l_color_texture, "l_color_texture");
    // Uncomment to use fog.
    /* material->add(*state->l_depth_texture, "l_depth_texture"); */
    material->add(*state->blur2_texture, "bloom_texture");
  }

  // Axes
  {
#if 0
    real32 axes_vertices[] = AXES_VERTICES;
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      axes_vertices, LEN(axes_vertices),
      nullptr, 0,
      "axes",
      GL_LINES,
      RenderPass::forward_nodepth,
      state->entity_manager.add("axes")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(0.0f, 0.1f, 0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(1.0f, 1.0f, 1.0f)
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "axes", ShaderType::standard,
      SHADER_DIR"axes.vert", SHADER_DIR"axes.frag", nullptr
    );
#endif
  }

  // Lights
  {
#if 1
    glm::vec3 light_direction = glm::vec3(
      sin(state->dir_light_angle), -sin(state->dir_light_angle), 0.0f
    );

    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::file,
      "sun",
      MODEL_DIR"cube.obj",
      RenderPass::forward_nodepth,
      state->entity_manager.add("sun")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(-light_direction * DIRECTIONAL_LIGHT_DISTANCE),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.3f)
    );
    model_asset->light_component = LightComponent(
      Entity::no_entity_handle,
      LightType::directional,
      light_direction,
      glm::vec4(4.0f, 4.0f, 4.0f, 1.0f),
      glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "light", ShaderType::standard,
      SHADER_DIR"simple.vert", SHADER_DIR"simple.frag", nullptr
    );
#endif

#if 0
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::file,
      "llightight",
      MODEL_DIR"cube.obj",
      RenderPass::forward_nodepth,
      state->entity_manager.add("pointlight")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(-7.0f, 3.0f, 0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(0.3f)
    );

    model_asset->light_component = LightComponent(
      Entity::no_entity_handle,
      LightType::point,
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec4(200.0f, 0.0f, 0.0f, 1.0f),
      /* glm::vec4(1.0f, 0.09f, 0.032f, 0.0f) */
      glm::vec4(1.0f, 0.09f, 0.032f, 0.0f) * 2.0f
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "pointlight", ShaderType::standard,
      SHADER_DIR"simple.vert", SHADER_DIR"simple.frag", nullptr
    );
#endif
  }

  // Ocean
  {
#if 1
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
      ModelSource::data,
      vertex_data, n_vertices,
      index_data, n_indices,
      "ocean",
      GL_TRIANGLES,
      RenderPass::forward_depth,
      state->entity_manager.add("ocean")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(1.0f)
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "water", ShaderType::standard,
      SHADER_DIR"water.vert", SHADER_DIR"water.frag",
      nullptr
    );
    material->depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "water_depth", ShaderType::depth,
      SHADER_DIR"water_depth.vert", SHADER_DIR"standard_depth.frag",
      nullptr
    );
    material->add(*state->g_position_texture, "g_position_texture");
    material->add(*state->g_albedo_texture, "g_albedo_texture");
    material->add(Texture(TextureType::normal, "resources/textures/vlachos.jpg"), "normal_texture");
    material->add(Texture(TextureType::other, "resources/textures/water_foam.png"), "foam_texture");
    material->add(
      Texture(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        TextureType::shadowmap, state->cube_shadowmaps,
        state->cube_shadowmap_width, state->cube_shadowmap_height, 1
      ),
      "cube_shadowmaps"
    );
    material->add(
      Texture(
        GL_TEXTURE_2D_ARRAY,
        TextureType::shadowmap, state->texture_shadowmaps,
        state->texture_shadowmap_width, state->texture_shadowmap_height, 1
      ),
      "texture_shadowmaps"
    );
    memory->temp_memory_pool.reset();
#endif
  }

  // Temple
  {
#if 0
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::file,
      "temple",
      MODEL_DIR"shop.fbx",
      RenderPass::deferred | RenderPass::shadowcaster,
      state->entity_manager.add("temple_root")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(0.0f, 0.1f, 0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      glm::vec3(0.1f)
    );

    {
      material = new(model_asset->materials.push()) Material(memory);
      material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory, "entity", ShaderType::standard,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
      );
      material->add(
        Texture(TextureType::albedo, "resources/textures/shop/03_-_Default_BaseColor.tga.png"),
        "albedo_texture"
      );
      material->add(
        Texture(TextureType::metallic, "resources/textures/shop/03_-_Default_Metallic.tga.png"),
        "metallic_texture"
      );
      material->add(
        Texture(TextureType::roughness, "resources/textures/shop/03_-_Default_Roughness.tga.png"),
        "roughness_texture"
      );
      material->add(
        Texture(TextureType::ao, "resources/textures/shop/AO-3.tga.png"),
        "ao_texture"
      );
      material->add(
        Texture(TextureType::normal, "resources/textures/shop/03_-_Default_Normal.tga.png"),
        "normal_texture"
      );
    }

    {
      material = new(model_asset->materials.push()) Material(memory);
      material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory, "entity", ShaderType::standard,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
      );
      material->add(
        Texture(TextureType::albedo, "resources/textures/shop/01_-_Default_BaseColor.tga.png"),
        "albedo_texture"
      );
      material->add(
        Texture(TextureType::metallic, "resources/textures/shop/01_-_Default_Metallic.tga.png"),
        "metallic_texture"
      );
      material->add(
        Texture(TextureType::roughness, "resources/textures/shop/01_-_Default_Roughness.tga.png"),
        "roughness_texture"
      );
      material->add(
        Texture(TextureType::ao, "resources/textures/shop/AO-1.tga.png"),
        "ao_texture"
      );
      material->add(
        Texture(TextureType::normal, "resources/textures/shop/01_-_Default_Normal.tga.png"),
        "normal_texture"
      );
    }

    {
      material = new(model_asset->materials.push()) Material(memory);
      material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory, "entity", ShaderType::standard,
        SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
      );
      material->add(
        Texture(TextureType::albedo, "resources/textures/shop/02_-_Default_BaseColor.tga.png"),
        "albedo_texture"
      );
      material->add(
        Texture(TextureType::metallic, "resources/textures/shop/02_-_Default_Metallic.tga.png"),
        "metallic_texture"
      );
      material->add(
        Texture(TextureType::roughness, "resources/textures/shop/02_-_Default_Roughness.tga.png"),
        "roughness_texture"
      );
      material->add(
        Texture(TextureType::ao, "resources/textures/shop/AO-2.tga.png"),
        "ao_texture"
      );
      material->add(
        Texture(TextureType::normal, "resources/textures/shop/02_-_Default_Normal.tga.png"),
        "normal_texture"
      );
    }
#endif
  }

  // Test
  EntityHandle test_entity_handle;
  {
    Entity *entity = state->entity_manager.add("test");
    test_entity_handle = entity->handle;
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::file,
      "test",
      MODEL_DIR"cube.obj",
      RenderPass::forward_depth | RenderPass::shadowcaster,
      entity
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      glm::vec3(1.0f)
    );

    model_asset->behavior_component = BehaviorComponent(
      Entity::no_entity_handle,
      Behavior::test
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "light", ShaderType::standard,
      SHADER_DIR"simple.vert", SHADER_DIR"simple.frag", nullptr
    );
  }

  // Rocks
  {
#if 1
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::file,
      "rocks",
      MODEL_DIR"Stones_AssetKit.fbx",
      RenderPass::deferred | RenderPass::shadowcaster,
      state->entity_manager.add("rocks_root")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(0.0f, -3.5f, 0.0f),
      glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      glm::vec3(0.05f),
      test_entity_handle
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "rocks", ShaderType::standard,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
    );
    material->add(
      Texture(TextureType::albedo, "resources/textures/rocks/2k/Stones_Color.jpg"),
      "albedo_texture"
    );
    material->set_metallic_static(0.0f);
    material->add(
      Texture(TextureType::roughness, "resources/textures/rocks/2k/Stones_Roughness.jpg"),
      "roughness_texture"
    );
    material->set_ao_static(1.0f);
    material->add(
      Texture(TextureType::normal, "resources/textures/rocks/2k/Stones_Normal.jpg"),
      "normal_texture"
    );
#endif
  }

  // Skysphere
  {
#if 1
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
      ModelSource::data,
      vertex_data, n_vertices,
      index_data, n_indices,
      "skysphere",
      GL_TRIANGLE_STRIP,
      RenderPass::forward_skybox,
      state->entity_manager.add("skysphere")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(75.0f)
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "skysphere", ShaderType::standard,
      SHADER_DIR"skysphere.vert", SHADER_DIR"skysphere.frag", nullptr
    );
    memory->temp_memory_pool.reset();
#endif
  }

  // Goose
  {
#if 0
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::file,
      "goose",
      MODEL_DIR"miniGoose.fbx",
      RenderPass::deferred,
      state->entity_manager.add("goose")
    );

    model_asset->spatial_component = SpatialComponent(
      Entity::no_entity_handle,
      glm::vec3(-4.6f, 0.00f, -1.5f),
      glm::angleAxis(glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      glm::vec3(0.2f)
    );

    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "entity", ShaderType::standard,
      SHADER_DIR"standard.vert", SHADER_DIR"standard.frag", nullptr
    );
    material->set_albedo_static(glm::vec4(0.6f, 0.1f, 0.1f, 1.0f));
    material->set_metallic_static(0.0f);
    material->set_roughness_static(1.0f);
    material->set_ao_static(1.0f);
#endif
  }
}
