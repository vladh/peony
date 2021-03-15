void World::get_entity_text_representation(
  char *text,
  State *state,
  Entity *entity,
  uint8 depth
) {
  EntityHandle handle = entity->handle;
  SpatialComponent *spatial_component =
    EntitySets::get_spatial_component_from_set(&state->spatial_component_set, handle);

  // Children will be drawn under their parents.
  if (
    depth == 0 &&
    Entities::is_spatial_component_valid(spatial_component) &&
    spatial_component->parent_entity_handle != Entity::no_entity_handle
  ) {
    return;
  }

  bool32 has_spatial_component = Entities::is_spatial_component_valid(
    spatial_component
  );
  bool32 has_drawable_component = Entities::is_drawable_component_valid(
    EntitySets::get_drawable_component_from_set(
      &state->drawable_component_set,
      handle
    )
  );
  bool32 has_light_component = Entities::is_light_component_valid(
    EntitySets::get_light_component_from_set(
      &state->light_component_set,
      handle
    )
  );
  bool32 has_behavior_component = Entities::is_behavior_component_valid(
    EntitySets::get_behavior_component_from_set(
      &state->behavior_component_set,
      handle
    )
  );

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

  if (Entities::is_spatial_component_valid(spatial_component)) {
    // NOTE: This is super slow lol.
    uint32 n_children_found = 0;
    for (
      uint32 child_idx = 1;
      child_idx < state->spatial_component_set.components->size;
      child_idx++
    ) {
      SpatialComponent *child_spatial_component =
        state->spatial_component_set.components->get(child_idx);
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
  for (uint32 idx = 0; idx < state->light_component_set.components->size; idx++) {
    LightComponent *light_component =
      state->light_component_set.components->get(idx);
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
  EntityTemplate *entity_template,
  MemoryPool *asset_memory_pool,
  EntitySet *entity_set,
  Array<ModelAsset> *model_assets,
  Array<ShaderAsset> *shader_assets,
  State *state
) {
  // TODO: Make `init_entity()` function and use it here like
  // we did with the other ones.
  // TODO: Figure out parent system.
  Entity *entity = EntitySets::add_entity_to_set(
    entity_set,
    entity_template->entity_debug_name
  );

  ModelAsset *model_asset = nullptr;

  if (strlen(entity_template->builtin_model_name) == 0) {
    model_asset = Models::init_model_asset(
      (ModelAsset*)(model_assets->push()),
      asset_memory_pool,
      ModelSource::file,
      entity_template->entity_debug_name,
      entity_template->model_path,
      entity_template->render_pass,
      entity->handle
    );
  } else {
    if (strcmp(entity_template->builtin_model_name, "axes") == 0) {
      model_asset = Models::init_model_asset(
        (ModelAsset*)(model_assets->push()),
        asset_memory_pool,
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
        asset_memory_pool,
        200, 200,
        800, 800,
        &ocean_n_vertices, &ocean_n_indices,
        &ocean_vertex_data, &ocean_index_data
      );

      model_asset = Models::init_model_asset(
        (ModelAsset*)(model_assets->push()),
        asset_memory_pool,
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

  if (Entities::is_spatial_component_valid(&entity_template->spatial_component)) {
    Entities::init_spatial_component(
      &model_asset->spatial_component,
      entity->handle,
      entity_template->spatial_component.position,
      entity_template->spatial_component.rotation,
      entity_template->spatial_component.scale
      // TODO: Add support for parents! Forgot about this.
    );
  }

  if (Entities::is_light_component_valid(&entity_template->light_component)) {
    Entities::init_light_component(
      &model_asset->light_component,
      entity->handle,
      entity_template->light_component.type,
      entity_template->light_component.direction,
      entity_template->light_component.color,
      entity_template->light_component.attenuation
    );
  }

  if (Entities::is_behavior_component_valid(&entity_template->behavior_component)) {
    Entities::init_behavior_component(
      &model_asset->behavior_component,
      entity->handle,
      entity_template->behavior_component.behavior
    );
  }

  for (
    uint32 idx_material = 0;
    idx_material < entity_template->n_materials;
    idx_material++
  ) {
    MaterialTemplate *material_template =
      &entity_template->material_templates[idx_material];
    Material *material = Materials::init_material(
      model_asset->materials.push(), asset_memory_pool
    );

    if (strlen(material_template->shader_asset_vert_path) > 0) {
      material->shader_asset = Shaders::init_shader_asset(
        (ShaderAsset*)(shader_assets->push()),
        entity_template->entity_debug_name,
        ShaderType::standard,
        material_template->shader_asset_vert_path,
        material_template->shader_asset_frag_path,
        material_template->shader_asset_geom_path
      );
    }
    if (strlen(material_template->depth_shader_asset_vert_path) > 0) {
      material->depth_shader_asset = Shaders::init_shader_asset(
        (ShaderAsset*)(shader_assets->push()),
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
      Texture texture;
      Materials::init_texture(
        &texture,
        material_template->texture_types[idx_texture],
        material_template->texture_paths[idx_texture]
      );
      Materials::add_texture_to_material(
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
        Materials::add_texture_to_material(
          material, *state->g_position_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "g_albedo_texture") == 0) {
        Materials::add_texture_to_material(
          material, *state->g_albedo_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "cube_shadowmaps") == 0) {
        Materials::add_texture_to_material(
          material, *state->cube_shadowmaps_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "texture_shadowmaps") == 0) {
        Materials::add_texture_to_material(
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


void World::create_internal_entities(MemoryPool *memory_pool, State *state) {
  ModelAsset *model_asset;
  Material *material;

  state->standard_depth_shader_asset = Shaders::init_shader_asset(
    (ShaderAsset*)(state->shader_assets.push()),
    "standard_depth", ShaderType::depth,
    "standard_depth.vert", "standard_depth.frag",
    "standard_depth.geom"
  );

  uint32 skysphere_n_vertices;
  uint32 skysphere_n_indices;
  real32 *skysphere_vertex_data;
  uint32 *skysphere_index_data;

  Util::make_sphere(
    memory_pool,
    64, 64,
    &skysphere_n_vertices, &skysphere_n_indices,
    &skysphere_vertex_data, &skysphere_index_data
  );

  // Lighting screenquad
  model_asset = Models::init_model_asset(
    (ModelAsset*)(state->model_assets.push()),
    memory_pool,
    ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_lighting",
    GL_TRIANGLES,
    RenderPass::lighting,
    EntitySets::add_entity_to_set(&state->entity_set, "screenquad_lighting")->handle
  );
  material = Materials::init_material(model_asset->materials.push(), memory_pool);
  material->shader_asset = Shaders::init_shader_asset(
    (ShaderAsset*)(state->shader_assets.push()),
    "lighting", ShaderType::standard,
    "lighting.vert", "lighting.frag", ""
  );
  Materials::add_texture_to_material(
    material, *state->g_position_texture, "g_position_texture"
  );
  Materials::add_texture_to_material(
    material, *state->g_normal_texture, "g_normal_texture"
  );
  Materials::add_texture_to_material(
    material, *state->g_albedo_texture, "g_albedo_texture"
  );
  Materials::add_texture_to_material(
    material, *state->g_pbr_texture, "g_pbr_texture"
  );
  Materials::add_texture_to_material(
    material, *state->cube_shadowmaps_texture, "cube_shadowmaps"
  );
  Materials::add_texture_to_material(
    material, *state->texture_shadowmaps_texture, "texture_shadowmaps"
  );

  // Preblur screenquad
  model_asset = Models::init_model_asset(
    (ModelAsset*)(state->model_assets.push()),
    memory_pool,
    ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_preblur",
    GL_TRIANGLES,
    RenderPass::preblur,
    EntitySets::add_entity_to_set(&state->entity_set, "screenquad_preblur")->handle
  );
  material = Materials::init_material(model_asset->materials.push(), memory_pool);
  material->shader_asset = Shaders::init_shader_asset(
    (ShaderAsset*)(state->shader_assets.push()),
    "blur", ShaderType::standard,
    "blur.vert", "blur.frag", ""
  );
  Materials::add_texture_to_material(
    material, *state->l_bright_color_texture, "source_texture"
  );

  // Blur 1 screenquad
  model_asset = Models::init_model_asset(
    (ModelAsset*)(state->model_assets.push()),
    memory_pool,
    ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_blur1",
    GL_TRIANGLES,
    RenderPass::blur1,
    EntitySets::add_entity_to_set(&state->entity_set, "screenquad_blur1")->handle
  );
  material = Materials::init_material(model_asset->materials.push(), memory_pool);
  material->shader_asset = Shaders::init_shader_asset(
    (ShaderAsset*)(state->shader_assets.push()),
    "blur", ShaderType::standard,
    "blur.vert", "blur.frag", ""
  );
  Materials::add_texture_to_material(material, *state->blur2_texture, "source_texture");

  // Blur 2 screenquad
  model_asset = Models::init_model_asset(
    (ModelAsset*)(state->model_assets.push()),
    memory_pool,
    ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_blur2",
    GL_TRIANGLES,
    RenderPass::blur2,
    EntitySets::add_entity_to_set(&state->entity_set, "screenquad_blur2")->handle
  );
  material = Materials::init_material(model_asset->materials.push(), memory_pool);
  material->shader_asset = Shaders::init_shader_asset(
    (ShaderAsset*)(state->shader_assets.push()),
    "blur", ShaderType::standard,
    "blur.vert", "blur.frag", ""
  );
  Materials::add_texture_to_material(material, *state->blur1_texture, "source_texture");

  // Postprocessing screenquad
  model_asset = Models::init_model_asset(
    (ModelAsset*)(state->model_assets.push()),
    memory_pool,
    ModelSource::data,
    (real32*)SCREENQUAD_VERTICES, 6,
    nullptr, 0,
    "screenquad_postprocessing",
    GL_TRIANGLES,
    RenderPass::postprocessing,
    EntitySets::add_entity_to_set(&state->entity_set, "screenquad_postprocessing")->handle
  );
  material = Materials::init_material(model_asset->materials.push(), memory_pool);
  material->shader_asset = Shaders::init_shader_asset(
    (ShaderAsset*)(state->shader_assets.push()),
    "postprocessing", ShaderType::standard,
    "postprocessing.vert", "postprocessing.frag", ""
  );
  Materials::add_texture_to_material(
    material, *state->l_color_texture, "l_color_texture"
  );
  Materials::add_texture_to_material(material, *state->blur2_texture, "bloom_texture");
  // Uncomment to use fog.
  /* Materials::add_texture_to-material( */
  /*   material, *state->l_depth_texture, "l_depth_texture" */
  /* ); */

  // Skysphere
  {
#if 1
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "skysphere"
    );

    model_asset = Models::init_model_asset(
      (ModelAsset*)(state->model_assets.push()),
      memory_pool,
      ModelSource::data,
      skysphere_vertex_data, skysphere_n_vertices,
      skysphere_index_data, skysphere_n_indices,
      "skysphere",
      GL_TRIANGLE_STRIP,
      RenderPass::forward_skybox,
      entity->handle
    );

    Entities::init_spatial_component(
      &model_asset->spatial_component,
      entity->handle,
      glm::vec3(0.0f),
      glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      glm::vec3(75.0f)
    );

    material = Materials::init_material(model_asset->materials.push(), memory_pool);
    material->shader_asset = Shaders::init_shader_asset(
      (ShaderAsset*)(state->shader_assets.push()),
      "skysphere", ShaderType::standard,
      "skysphere.vert", "skysphere.frag", ""
    );
#endif
  }
}


void World::init(
  MemoryPool *asset_memory_pool,
  MemoryPool *entity_memory_pool,
  State *state
) {
  MemoryPool temp_memory_pool = {};
  create_internal_entities(entity_memory_pool, state);

  EntityTemplate *entity_templates =
    (EntityTemplate*)Memory::push(
      &temp_memory_pool,
      sizeof(EntityTemplate) * PeonyFileParser::MAX_N_FILE_ENTRIES,
      "entity_templates"
    );

  uint32 n_entities = PeonyFileParser::parse_scene_file(
    "data/scenes/demo.peony_scene", entity_templates
  );

  for (uint32 idx_entity = 0; idx_entity < n_entities; idx_entity++) {
    /* PeonyFileParser::print_entity_template(&entity_templates[idx_entity]); */
    create_entities_from_entity_template(
      &entity_templates[idx_entity],
      asset_memory_pool,
      &state->entity_set,
      &state->model_assets,
      &state->shader_assets,
      state
    );
  }

  Memory::destroy_memory_pool(&temp_memory_pool);
}


void World::check_all_model_assets_loaded(State *state) {
  for (uint32 idx = 0; idx < state->model_assets.size; idx++) {
    ModelAsset *model_asset = state->model_assets[idx];
    Models::prepare_for_draw(
      model_asset,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue,
      &state->entity_set,
      &state->drawable_component_set,
      &state->spatial_component_set,
      &state->light_component_set,
      &state->behavior_component_set
    );
  }
}


void World::update(State *state) {
  Cameras::update_matrices(
    state->camera_active,
    state->window_info.width,
    state->window_info.height
  );
  check_all_model_assets_loaded(state);

  for (
    uint32 idx = 1;
    idx < state->behavior_component_set.components->size;
    idx++
  ) {
    BehaviorComponent *behavior_component =
      state->behavior_component_set.components->get(idx);

    if (
      !behavior_component ||
      !Entities::is_behavior_component_valid(behavior_component)
    ) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    SpatialComponent *spatial_component =
      EntitySets::get_spatial_component_from_set(
        &state->spatial_component_set, entity_handle
      );
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
    for (uint32 idx = 0; idx < state->light_component_set.components->size; idx++) {
      LightComponent *light_component =
        state->light_component_set.components->get(idx);
      SpatialComponent *spatial_component =
        EntitySets::get_spatial_component_from_set(
          &state->spatial_component_set,
          light_component->entity_handle
        );

      if (!(
        Entities::is_light_component_valid(light_component) &&
        Entities::is_spatial_component_valid(spatial_component)
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
          -light_component->direction * Renderer::DIRECTIONAL_LIGHT_DISTANCE;
      }
    }
  }
}
