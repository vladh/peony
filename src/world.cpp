void World::get_entity_text_representation(
  char *text, State *state, Entity *entity, uint8 depth
) {
  EntityHandle handle = entity->handle;
  SpatialComponent *spatial_component = state->spatial_component_manager.get(handle);

  // Children will be drawn under their parents.
  if (
    depth == 0 &&
    spatial_component->is_valid() &&
    spatial_component->parent_entity_handle != Entity::no_entity_handle
  ) {
    return;
  }

  bool32 has_spatial_component = spatial_component->is_valid();
  bool32 has_drawable_component =
    state->drawable_component_manager.get(handle)->is_valid();
  bool32 has_light_component =
    state->light_component_manager.get(handle)->is_valid();
  bool32 has_behavior_component =
    state->behavior_component_manager.get(handle)->is_valid();

  for (uint8 level = 0; level < depth; level++) {
    strcat(text, "  ");
  }

  strcat(text, "- ");
  strcat(text, entity->debug_name);

  strcat(text, "@");
  // Because NUM_TO_STR only has 2048 entries
  assert(entity->handle < 2048);
  strcat(text, NUM_TO_STR[entity->handle]);

  if (
    !has_spatial_component &&
    !has_drawable_component &&
    !has_light_component &&
    !has_behavior_component
  ) {
    strcat(text, " (orphan)");
  }

  if (has_spatial_component) {
    strcat(text, " +S");
  }
  if (has_drawable_component) {
    strcat(text, " +D");
  }
  if (has_light_component) {
    strcat(text, " +L");
  }
  if (has_behavior_component) {
    strcat(text, " +B");
  }

  if (spatial_component->is_valid()) {
    // NOTE: This is super slow lol.
    uint32 n_children_found = 0;
    for (
      uint32 child_idx = 1;
      child_idx < state->spatial_component_manager.components->size;
      child_idx++
    ) {
      SpatialComponent *child_spatial_component =
        state->spatial_component_manager.components->get(child_idx);
      if (
        child_spatial_component->parent_entity_handle ==
          spatial_component->entity_handle
      ) {
        n_children_found++;
        if (n_children_found > 5) {
          continue;
        }
        EntityHandle child_handle = child_spatial_component->entity_handle;
        Entity *child_entity = state->entities.get(child_handle);

        if (text[strlen(text) - 1] != '\n') {
          strcat(text, "\n");
        }
        get_entity_text_representation(text, state, child_entity, depth + 1);
      }
    }
    if (n_children_found > 5) {
      for (uint8 level = 0; level < (depth + 1); level++) {
        strcat(text, "  ");
      }
      strcat(text, "(and ");
      strcat(text, NUM_TO_STR[n_children_found - 5]);
      strcat(text, " more)");
    }
  }

  if (text[strlen(text) - 1] != '\n') {
    strcat(text, "\n");
  }
}


void World::get_scene_text_representation(char *text, State *state) {
  strcpy(text, "");

  for (uint32 idx = 1; idx < state->entities.size; idx++) {
    Entity *entity = state->entities[idx];
    get_entity_text_representation(text, state, entity, 0);
  }

  if (text[strlen(text) - 1] == '\n') {
    text[strlen(text) - 1] = 0;
  }
}


void World::update_light_position(State *state, real32 amount) {
  for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
    LightComponent *light_component = state->light_component_manager.components->get(idx);
    if (light_component -> type == LightType::directional) {
      state->dir_light_angle += amount;
      light_component->direction = glm::vec3(
        sin(state->dir_light_angle), -cos(state->dir_light_angle), 0.0f
      );
      break;
    }
  }
}


void World::create_entities_from_entity_template(
  PeonyFileParser::EntityTemplate *entity_template,
  Memory *memory,
  EntityManager *entity_manager,
  Array<Models::ModelAsset> *model_assets,
  Array<ShaderAsset> *shader_assets,
  State *state
) {
  Entity *entity = entity_manager->add(entity_template->entity_debug_name);

  Models::ModelAsset *model_asset = nullptr;

  if (strlen(entity_template->builtin_model_name) == 0) {
    model_asset = Models::init_model_asset(
      (Models::ModelAsset*)(model_assets->push()),
      memory,
      Models::ModelSource::file,
      entity_template->entity_debug_name,
      entity_template->model_path,
      entity_template->render_pass,
      entity->handle
    );
  } else {
    if (strcmp(entity_template->builtin_model_name, "axes") == 0) {
      model_asset = Models::init_model_asset(
        (Models::ModelAsset*)(model_assets->push()),
        memory,
        Models::ModelSource::data,
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

      model_asset = Models::init_model_asset(
        (Models::ModelAsset*)(model_assets->push()),
        memory,
        Models::ModelSource::data,
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
    Textures::Material *material = Textures::init_material(
      model_asset->materials.push(), memory
    );

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

    material->albedo_static = material_template->albedo_static;
    material->metallic_static = material_template->metallic_static;
    material->roughness_static = material_template->roughness_static;
    material->ao_static = material_template->ao_static;

    for (
      uint32 idx_texture = 0;
      idx_texture < material_template->n_textures;
      idx_texture++
    ) {
      Textures::Texture texture;
      Textures::init_texture(
        &texture,
        material_template->texture_types[idx_texture],
        material_template->texture_paths[idx_texture]
      );
      Textures::add_texture_to_material(
        material,
        texture,
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
        Textures::add_texture_to_material(
          material, *state->g_position_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "g_albedo_texture") == 0) {
        Textures::add_texture_to_material(
          material, *state->g_albedo_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "cube_shadowmaps") == 0) {
        Textures::add_texture_to_material(
          material, *state->cube_shadowmaps_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "texture_shadowmaps") == 0) {
        Textures::add_texture_to_material(
          material, *state->texture_shadowmaps_texture, builtin_texture_name
        );
      } else {
        log_fatal(
          "Attempted to use unsupported built-in texture %s",
          builtin_texture_name
        );
      }
    }
  }
}


void World::create_internal_entities(Memory *memory, State *state) {
  Models::ModelAsset *model_asset;
  Textures::Material *material;

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
  model_asset = Models::init_model_asset(
    (Models::ModelAsset*)(state->model_assets.push()),
    memory,
    Models::ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_lighting",
    GL_TRIANGLES,
    Renderer::RenderPass::lighting,
    state->entity_manager.add("screenquad_lighting")->handle
  );
  material = Textures::init_material(model_asset->materials.push(), memory);
  material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory, "lighting", ShaderType::standard,
    "lighting.vert", "lighting.frag", ""
  );
  Textures::add_texture_to_material(
    material, *state->g_position_texture, "g_position_texture"
  );
  Textures::add_texture_to_material(
    material, *state->g_normal_texture, "g_normal_texture"
  );
  Textures::add_texture_to_material(
    material, *state->g_albedo_texture, "g_albedo_texture"
  );
  Textures::add_texture_to_material(
    material, *state->g_pbr_texture, "g_pbr_texture"
  );
  Textures::add_texture_to_material(
    material, *state->cube_shadowmaps_texture, "cube_shadowmaps"
  );
  Textures::add_texture_to_material(
    material, *state->texture_shadowmaps_texture, "texture_shadowmaps"
  );

  // Preblur screenquad
  model_asset = Models::init_model_asset(
    (Models::ModelAsset*)(state->model_assets.push()),
    memory,
    Models::ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_preblur",
    GL_TRIANGLES,
    Renderer::RenderPass::preblur,
    state->entity_manager.add("screenquad_preblur")->handle
  );
  material = Textures::init_material(model_asset->materials.push(), memory);
  material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory, "blur", ShaderType::standard,
    "blur.vert", "blur.frag", ""
  );
  Textures::add_texture_to_material(
    material, *state->l_bright_color_texture, "source_texture"
  );

  // Blur 1 screenquad
  model_asset = Models::init_model_asset(
    (Models::ModelAsset*)(state->model_assets.push()),
    memory,
    Models::ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_blur1",
    GL_TRIANGLES,
    Renderer::RenderPass::blur1,
    state->entity_manager.add("screenquad_blur1")->handle
  );
  material = Textures::init_material(model_asset->materials.push(), memory);
  material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory, "blur", ShaderType::standard,
    "blur.vert", "blur.frag", ""
  );
  Textures::add_texture_to_material(material, *state->blur2_texture, "source_texture");

  // Blur 2 screenquad
  model_asset = Models::init_model_asset(
    (Models::ModelAsset*)(state->model_assets.push()),
    memory,
    Models::ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_blur2",
    GL_TRIANGLES,
    Renderer::RenderPass::blur2,
    state->entity_manager.add("screenquad_blur2")->handle
  );
  material = Textures::init_material(model_asset->materials.push(), memory);
  material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory, "blur", ShaderType::standard,
    "blur.vert", "blur.frag", ""
  );
  Textures::add_texture_to_material(material, *state->blur1_texture, "source_texture");

  // Postprocessing screenquad
  model_asset = Models::init_model_asset(
    (Models::ModelAsset*)(state->model_assets.push()),
    memory,
    Models::ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_postprocessing",
    GL_TRIANGLES,
    Renderer::RenderPass::postprocessing,
    state->entity_manager.add("screenquad_postprocessing")->handle
  );
  material = Textures::init_material(model_asset->materials.push(), memory);
  material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory, "postprocessing", ShaderType::standard,
    "postprocessing.vert", "postprocessing.frag", ""
  );
  Textures::add_texture_to_material(
    material, *state->l_color_texture, "l_color_texture"
  );
  Textures::add_texture_to_material(material, *state->blur2_texture, "bloom_texture");
  // Uncomment to use fog.
  /* Textures::add_texture_to-material( */
  /*   material, *state->l_depth_texture, "l_depth_texture" */
  /* ); */

  // Skysphere
  {
#if 1
    Entity *entity = state->entity_manager.add("skysphere");

    model_asset = Models::init_model_asset(
      (Models::ModelAsset*)(state->model_assets.push()),
      memory,
      Models::ModelSource::data,
      skysphere_vertex_data, skysphere_n_vertices,
      skysphere_index_data, skysphere_n_indices,
      "skysphere",
      GL_TRIANGLE_STRIP,
      Renderer::RenderPass::forward_skybox,
      entity->handle
    );

    model_asset->spatial_component = SpatialComponent(
      entity->handle,
      glm::vec3(0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(75.0f)
    );

    material = Textures::init_material(model_asset->materials.push(), memory);
    material->shader_asset = new(state->shader_assets.push()) ShaderAsset(
      memory, "skysphere", ShaderType::standard,
      "skysphere.vert", "skysphere.frag", ""
    );
#endif
  }
}


void World::init(Memory *memory, State *state) {
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


void World::check_all_model_assets_loaded(Memory *memory, State *state) {
  for (uint32 idx = 0; idx < state->model_assets.size; idx++) {
    Models::ModelAsset *model_asset = state->model_assets[idx];
    Models::prepare_for_draw(
      model_asset,
      memory,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue,
      &state->entity_manager,
      &state->drawable_component_manager,
      &state->spatial_component_manager,
      &state->light_component_manager,
      &state->behavior_component_manager
    );
  }
}


void World::update(Memory *memory, State *state) {
  Cameras::update_matrices(
    state->camera_active,
    state->window_info.width,
    state->window_info.height
  );
  check_all_model_assets_loaded(memory, state);

  for (
    uint32 idx = 1;
    idx < state->behavior_component_manager.components->size;
    idx++
  ) {
    BehaviorComponent *behavior_component =
      state->behavior_component_manager.components->get(idx);

    if (
      !behavior_component ||
      !behavior_component->is_valid()
    ) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    SpatialComponent *spatial_component =
      state->spatial_component_manager.get(entity_handle);
    if (!spatial_component) {
      log_error("Could not get SpatialComponent for BehaviorComponent");
      continue;
    }

    Entity *entity = state->entities.get(entity_handle);
    if (!entity) {
      log_error("Could not get Entity for BehaviorComponent");
      continue;
    }

    if (behavior_component->behavior == Behavior::test) {
      spatial_component->position = glm::vec3(
        (real32)sin(state->t) * 15.0f,
        (real32)((sin(state->t * 2.0f) + 1.5) * 3.0f),
        (real32)cos(state->t) * 15.0f
      );
    }
  }

  {
    for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
      LightComponent *light_component =
        state->light_component_manager.components->get(idx);
      SpatialComponent *spatial_component = state->spatial_component_manager.get(
        light_component->entity_handle
      );

      if (!(
        light_component->is_valid() &&
        spatial_component->is_valid()
      )) {
        continue;
      }

      if (light_component->type == LightType::point) {
        real64 time_term =
          (sin(state->t / 1.5f) + 1.0f) / 2.0f * (PI / 2.0f) + (PI / 2.0f);
        real64 x_term = 0.0f + cos(time_term) * 8.0f;
        real64 z_term = 0.0f + sin(time_term) * 8.0f;
        spatial_component->position.x = (real32)x_term;
        spatial_component->position.z = (real32)z_term;
      } else if (light_component->type == LightType::directional) {
        spatial_component->position = state->camera_active->position +
          -light_component->direction * DIRECTIONAL_LIGHT_DISTANCE;
      }
    }
  }
}
