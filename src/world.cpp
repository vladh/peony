namespace World {
  void create_entities_from_entity_template(
    PeonyFileParser::EntityTemplate *entity_template,
    Memory *memory,
    EntityManager *entity_manager,
    Array<ModelAsset> *model_assets,
    Array<ShaderAsset> *shader_assets,
    State *state
  ) {
    Entity *entity = entity_manager->add(entity_template->entity_debug_name);

    ModelAsset *model_asset = nullptr;

    if (strlen(entity_template->builtin_model_name) == 0) {
      model_asset = new(model_assets->push()) ModelAsset(
        memory,
        ModelSource::file,
        entity_template->entity_debug_name,
        entity_template->model_path,
        entity_template->render_pass,
        entity->handle
      );
    } else {
      if (strcmp(entity_template->builtin_model_name, "axes") == 0) {
        model_asset = new(model_assets->push()) ModelAsset(
          memory,
          ModelSource::data,
          (real32*)AXES_VERTICES, 6,
          nullptr, 0,
          entity_template->entity_debug_name,
          GL_LINES,
          entity_template->render_pass,
          entity->handle
        );
      } else if (strcmp(entity_template->builtin_model_name, "ocean") == 0) {
        uint32 ocean_n_vertices;
        uint32 ocean_n_indices;
        real32 *ocean_vertex_data;
        uint32 *ocean_index_data;

        Util::make_plane(
          &memory->asset_memory_pool,
          200, 200,
          800, 800,
          &ocean_n_vertices, &ocean_n_indices,
          &ocean_vertex_data, &ocean_index_data
        );

        model_asset = new(model_assets->push()) ModelAsset(
          memory,
          ModelSource::data,
          ocean_vertex_data, ocean_n_vertices,
          ocean_index_data, ocean_n_indices,
          entity_template->entity_debug_name,
          GL_TRIANGLES,
          entity_template->render_pass,
          entity->handle
        );
      } else {
        log_fatal(
          "Could not find builtin model: %s", entity_template->builtin_model_name
        );
      }
    }

    if (!model_asset) {
      log_fatal("Found no model asset.");
    }

    if (entity_template->spatial_component.is_valid()) {
      model_asset->spatial_component = SpatialComponent(
        entity->handle,
        entity_template->spatial_component.position,
        entity_template->spatial_component.rotation,
        entity_template->spatial_component.scale
        // TODO: Add support for parents! Forgot about this.
      );
    }

    if (entity_template->light_component.is_valid()) {
      model_asset->light_component = LightComponent(
        entity->handle,
        entity_template->light_component.type,
        entity_template->light_component.direction,
        entity_template->light_component.color,
        entity_template->light_component.attenuation
      );
    }

    if (entity_template->behavior_component.is_valid()) {
      model_asset->behavior_component = BehaviorComponent(
        entity->handle,
        entity_template->behavior_component.behavior
      );
    }

    for (
      uint32 idx_material = 0;
      idx_material < entity_template->n_materials;
      idx_material++
    ) {
      PeonyFileParser::MaterialTemplate *material_template =
        &entity_template->material_templates[idx_material];
      Material *material = new(model_asset->materials.push()) Material(memory);

      if (strlen(material_template->shader_asset_vert_path) > 0) {
        material->shader_asset = new(shader_assets->push()) ShaderAsset(
          memory,
          entity_template->entity_debug_name,
          ShaderType::standard,
          material_template->shader_asset_vert_path,
          material_template->shader_asset_frag_path,
          material_template->shader_asset_geom_path
        );
      }
      if (strlen(material_template->depth_shader_asset_vert_path) > 0) {
        material->depth_shader_asset = new(shader_assets->push()) ShaderAsset(
          memory,
          entity_template->entity_debug_name,
          ShaderType::depth,
          material_template->depth_shader_asset_vert_path,
          material_template->depth_shader_asset_frag_path,
          material_template->depth_shader_asset_geom_path
        );
      }

      material->set_albedo_static(material_template->albedo_static);
      material->set_metallic_static(material_template->metallic_static);
      material->set_roughness_static(material_template->roughness_static);
      material->set_ao_static(material_template->ao_static);

      for (
        uint32 idx_texture = 0;
        idx_texture < material_template->n_textures;
        idx_texture++
      ) {
        material->add(
          Texture(
            material_template->texture_types[idx_texture],
            material_template->texture_paths[idx_texture]
          ),
          material_template->texture_uniform_names[idx_texture]
        );
      }

      for (
        uint32 idx_texture = 0;
        idx_texture < material_template->n_builtin_textures;
        idx_texture++
      ) {
        const char *builtin_texture_name =
          material_template->builtin_texture_names[idx_texture];
        // TODO: Make the built-in textures some kind of array, that we can
        // also pass in instead of passing State.
        // NOTE: This list is intentionally not complete until we fix the above.
        if (strcmp(builtin_texture_name, "g_position_texture") == 0) {
          material->add(*state->g_position_texture, builtin_texture_name);
        } else if (strcmp(builtin_texture_name, "g_albedo_texture") == 0) {
          material->add(*state->g_albedo_texture, builtin_texture_name);
        } else if (strcmp(builtin_texture_name, "cube_shadowmaps") == 0) {
          material->add(*state->cube_shadowmaps_texture, builtin_texture_name);
        } else if (strcmp(builtin_texture_name, "texture_shadowmaps") == 0) {
          material->add(*state->texture_shadowmaps_texture, builtin_texture_name);
        } else {
          log_fatal(
            "Attempted to use unsupported built-in texture %s",
            builtin_texture_name
          );
        }
      }
    }
  }


  void create_internal_entities(Memory *memory, State *state) {
    ModelAsset *model_asset;
    Material *material;

    state->standard_depth_shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "standard_depth", ShaderType::depth,
      "standard_depth.vert", "standard_depth.frag",
      "standard_depth.geom"
    );

    uint32 skysphere_n_vertices;
    uint32 skysphere_n_indices;
    real32 *skysphere_vertex_data;
    uint32 *skysphere_index_data;

    Util::make_sphere(
      &memory->asset_memory_pool,
      64, 64,
      &skysphere_n_vertices, &skysphere_n_indices,
      &skysphere_vertex_data, &skysphere_index_data
    );

    // Lighting screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      (real32*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_lighting",
      GL_TRIANGLES,
      RenderPass::lighting,
      state->entity_manager.add("screenquad_lighting")->handle
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "lighting", ShaderType::standard,
      "lighting.vert", "lighting.frag", ""
    );
    material->add(*state->g_position_texture, "g_position_texture");
    material->add(*state->g_normal_texture, "g_normal_texture");
    material->add(*state->g_albedo_texture, "g_albedo_texture");
    material->add(*state->g_pbr_texture, "g_pbr_texture");
    material->add(*state->cube_shadowmaps_texture, "cube_shadowmaps");
    material->add(*state->texture_shadowmaps_texture, "texture_shadowmaps");

    // Preblur screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      (real32*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_preblur",
      GL_TRIANGLES,
      RenderPass::preblur,
      state->entity_manager.add("screenquad_preblur")->handle
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "blur", ShaderType::standard,
      "blur.vert", "blur.frag", ""
    );
    material->add(*state->l_bright_color_texture, "source_texture");

    // Blur 1 screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      (real32*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_blur1",
      GL_TRIANGLES,
      RenderPass::blur1,
      state->entity_manager.add("screenquad_blur1")->handle
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "blur", ShaderType::standard,
      "blur.vert", "blur.frag", ""
    );
    material->add(*state->blur2_texture, "source_texture");

    // Blur 2 screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      (real32*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_blur2",
      GL_TRIANGLES,
      RenderPass::blur2,
      state->entity_manager.add("screenquad_blur2")->handle
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "blur", ShaderType::standard,
      "blur.vert", "blur.frag", ""
    );
    material->add(*state->blur1_texture, "source_texture");

    // Postprocessing screenquad
    model_asset = new(state->model_assets.push()) ModelAsset(
      memory,
      ModelSource::data,
      (real32*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_postprocessing",
      GL_TRIANGLES,
      RenderPass::postprocessing,
      state->entity_manager.add("screenquad_postprocessing")->handle
    );
    material = new(model_asset->materials.push()) Material(memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "postprocessing", ShaderType::standard,
      "postprocessing.vert", "postprocessing.frag", ""
    );
    material->add(*state->l_color_texture, "l_color_texture");
    // Uncomment to use fog.
    /* material->add(*state->l_depth_texture, "l_depth_texture"); */
    material->add(*state->blur2_texture, "bloom_texture");

    // Skysphere
    {
#if 1
      Entity *entity = state->entity_manager.add("skysphere");

      model_asset = new(state->model_assets.push()) ModelAsset(
        memory,
        ModelSource::data,
        skysphere_vertex_data, skysphere_n_vertices,
        skysphere_index_data, skysphere_n_indices,
        "skysphere",
        GL_TRIANGLE_STRIP,
        RenderPass::forward_skybox,
        entity->handle
      );

      model_asset->spatial_component = SpatialComponent(
        entity->handle,
        glm::vec3(0.0f),
        glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::vec3(75.0f)
      );

      material = new(model_asset->materials.push()) Material(memory);
      material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
        memory, "skysphere", ShaderType::standard,
        "skysphere.vert", "skysphere.frag", ""
      );
#endif
    }
  }


  void init(Memory *memory, State *state) {
    create_internal_entities(memory, state);

    PeonyFileParser::EntityTemplate *entity_templates =
      (PeonyFileParser::EntityTemplate*)memory->temp_memory_pool.push(
        sizeof(PeonyFileParser::EntityTemplate) *
          PeonyFileParser::MAX_N_FILE_ENTRIES,
        "entity_templates"
      );

    uint32 n_entities = PeonyFileParser::parse_scene_file(
      "data/scenes/demo.peony_scene", entity_templates
    );

    for (uint32 idx_entity = 0; idx_entity < n_entities; idx_entity++) {
      /* PeonyFileParser::print_entity_template(&entity_templates[idx_entity]); */
      create_entities_from_entity_template(
        &entity_templates[idx_entity],
        memory,
        &state->entity_manager,
        &state->model_assets,
        &state->shader_assets,
        state
      );
    }
  }
};
