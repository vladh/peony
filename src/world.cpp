void World::get_entity_text_representation(
  char *text,
  State *state,
  Entity *entity,
  uint8 depth
) {
  EntityHandle handle = entity->handle;
  SpatialComponent *spatial_component =
    state->spatial_component_set.components[handle];

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
  bool32 has_drawable_component = Models::is_drawable_component_valid(
    state->drawable_component_set.components[handle]
  );
  bool32 has_light_component = Entities::is_light_component_valid(
    state->light_component_set.components[handle]
  );
  bool32 has_behavior_component = Entities::is_behavior_component_valid(
    state->behavior_component_set.components[handle]
  );
  bool32 has_animation_component = Entities::is_animation_component_valid(
    state->animation_component_set.components[handle]
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
    !has_behavior_component &&
    !has_animation_component
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
  if (has_animation_component) {
    strcat(text, " +A");
  }

  if (Entities::is_spatial_component_valid(spatial_component)) {
    // NOTE: This is super slow lol.
    uint32 n_children_found = 0;
    for_each (child_spatial_component, state->spatial_component_set.components) {
      if (
        child_spatial_component->parent_entity_handle ==
          spatial_component->entity_handle
      ) {
        n_children_found++;
        if (n_children_found > 5) {
          continue;
        }
        EntityHandle child_handle = child_spatial_component->entity_handle;
        Entity *child_entity = state->entity_set.entities[child_handle];

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
  text[0] = '\0';

  for_each (entity, state->entity_set.entities) {
    get_entity_text_representation(text, state, entity, 0);
  }

  if (text[strlen(text) - 1] == '\n') {
    text[strlen(text) - 1] = '\0';
  }
}


void World::get_materials_text_representation(char *text, State *state) {
  text[0] = '\0';

  strcat(text, "Internal:\n");

  uint32 idx = 0;
  for_each (material, state->materials) {
    strcat(text, "- ");
    strcat(text, material->name);
    strcat(text, "\n");
    if (idx == state->first_non_internal_material_idx - 1) {
      strcat(text, "Non-internal: \n");
    }
    idx++;
  }

  if (text[strlen(text) - 1] == '\n') {
    text[strlen(text) - 1] = '\0';
  }
}


void World::update_light_position(State *state, real32 amount) {
  for_each (light_component, state->light_component_set.components) {
    if (light_component->type == LightType::directional) {
      state->dir_light_angle += amount;
      break;
    }
  }
}


void World::create_model_loader_from_entity_template(
  EntityTemplate *entity_template,
  EntityHandle entity_handle,
  Array<ModelLoader> *model_loaders,
  State *state
) {
  ModelLoader *model_loader = model_loaders->push();

  Models::init_model_loader(
    model_loader,
    entity_template->model_source,
    entity_template->model_path_or_builtin_model_name
  );

  assert(
    sizeof(model_loader->material_names) == sizeof(entity_template->material_names)
  );
  memcpy(
    &model_loader->material_names,
    &entity_template->material_names,
    sizeof(entity_template->material_names)
  );
}


void World::create_entity_loader_from_entity_template(
  EntityTemplate *entity_template,
  EntityHandle entity_handle,
  EntityLoaderSet *entity_loader_set,
  State *state
) {
  EntityLoader *entity_loader = entity_loader_set->loaders[entity_handle];
  Models::init_entity_loader(
    entity_loader,
    entity_template->entity_debug_name,
    entity_template->model_path_or_builtin_model_name,
    entity_template->render_pass,
    entity_handle
  );
  entity_loader->spatial_component = entity_template->spatial_component;
  entity_loader->light_component = entity_template->light_component;
  entity_loader->behavior_component = entity_template->behavior_component;
  entity_loader->physics_component = entity_template->physics_component;
}


void World::create_internal_materials(State *state) {
  MemoryPool temp_memory_pool = {};

  // unknown
  {
    Material *material = Materials::init_material(
      state->materials.push(), "unknown"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "unknown", ShaderType::standard,
      "base.vert", "unknown.frag", ""
    );
  }

  // lighting
  {
    Material *material = Materials::init_material(
      state->materials.push(), "lighting"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "lighting", ShaderType::standard,
      "screenquad.vert", "lighting.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "g_position_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
    );
  }

#if USE_BLOOM
  // preblur
  {
    Material *material = Materials::init_material(
      state->materials.push(), "preblur"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "screenquad.vert", "blur.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_bright_color_texture, "source_texture"
    );
  }

  // blur1
  {
    Material *material = Materials::init_material(
      state->materials.push(), "blur1"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "screenquad.vert", "blur.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.blur2_texture, "source_texture"
    );
  }

  // blur2
  {
    Material *material = Materials::init_material(
      state->materials.push(), "blur2"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "screenquad.vert", "blur.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.blur1_texture, "source_texture"
    );
  }
#endif

  // postprocessing
  {
    Material *material = Materials::init_material(
      state->materials.push(), "postprocessing"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "postprocessing", ShaderType::standard,
      "screenquad.vert", "postprocessing.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_color_texture"
    );
#if USE_BLOOM
    Materials::add_texture_to_material(
      material, *state->builtin_textures.blur2_texture, "bloom_texture"
    );
#endif
    // Uncomment to use fog.
    /* Materials::add_texture_to-material( */
    /*   material, *state->l_depth_texture, "l_depth_texture" */
    /* ); */
  }

  // renderdebug
  {
    Material *material = Materials::init_material(
      state->materials.push(), "renderdebug"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "renderdebug", ShaderType::standard,
      "screenquad.vert", "renderdebug.frag", ""
    );

    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "g_position_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
    );

    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "l_color_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_bright_color_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_depth_texture"
    );

    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "blur1_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "blur2_texture"
    );

    Materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
    );
  }

  // skysphere
  {
    Material *material = Materials::init_material(
      state->materials.push(), "skysphere"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "skysphere", ShaderType::standard,
      "skysphere.vert", "skysphere.frag", ""
    );
  }


  // We've created all internal materials, so we will mark the next position
  // in the array of materials, so we know where non-internal materials start.
  state->first_non_internal_material_idx = state->materials.length;

  Memory::destroy_memory_pool(&temp_memory_pool);
}


void World::create_internal_entities(State *state) {
  MemoryPool temp_memory_pool = {};

  Shaders::init_shader_asset(
    &state->standard_depth_shader_asset,
    &temp_memory_pool,
    "standard_depth", ShaderType::depth,
    "standard_depth.vert", "standard_depth.frag",
    "standard_depth.geom"
  );

  // Lighting screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_lighting"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader,
      ModelSource::data,
      "screenquad_lighting"
    );
    Models::init_entity_loader(
      entity_loader,
      "screenquad_lighting",
      "screenquad_lighting",
      RenderPass::lighting,
      entity->handle
    );
    strcpy(*(model_loader->material_names.push()), "lighting");
  }

#if USE_BLOOM
  // Preblur screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_preblur"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader
      entity_loader,
      ModelSource::data,
      "screenquad_preblur"
    );
    Models::init_entity_loader(
      entity_loader,
      "screenquad_preblur",
      "screenquad_preblur",
      RenderPass::preblur,
      entity->handle
    );
    strcpy(*(model_loader->material_names.push()), "preblur");
  }

  // Blur 1 screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_blur1"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader,
      ModelSource::data,
      "screenquad_blur1"
    );
    Models::init_entity_loader(
      entity_loader,
      "screenquad_blur1",
      "screenquad_blur1",
      RenderPass::blur1,
      entity->handle
    );
    strcpy(*(model_loader->material_names.push()), "blur1");
  }

  // Blur 2 screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_blur2"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader,
      ModelSource::data,
      "screenquad_blur2",
    );
    Models::init_entity_loader(
      entity_loader,
      "screenquad_blur2",
      "screenquad_blur2",
      RenderPass::blur2,
      entity->handle
    );
    strcpy(*(model_loader->material_names.push()), "blur2");
  }
#endif

  // Postprocessing screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_postprocessing"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader,
      ModelSource::data,
      "screenquad_postprocessing"
    );
    Models::init_entity_loader(
      entity_loader,
      "screenquad_postprocessing",
      "screenquad_postprocessing",
      RenderPass::postprocessing,
      entity->handle
    );
    strcpy(*(model_loader->material_names.push()), "postprocessing");
  }

  // Debug screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_renderdebug"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader,
      ModelSource::data,
      "screenquad_renderdebug"
    );
    Models::init_entity_loader(
      entity_loader,
      "screenquad_renderdebug",
      "screenquad_renderdebug",
      RenderPass::renderdebug,
      entity->handle
    );
    strcpy(*(model_loader->material_names.push()), "renderdebug");
  }

  // Skysphere
  {
#if 1
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "skysphere"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    Models::init_model_loader(
      model_loader,
      ModelSource::data,
      "skysphere"
    );
    Models::init_entity_loader(
      entity_loader,
      "skysphere",
      "skysphere",
      RenderPass::forward_skybox,
      entity->handle
    );
    entity_loader->spatial_component = {
      .entity_handle = entity->handle,
      .position = glm::vec3(0.0f),
      .rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      .scale = glm::vec3(75.0f),
    };
    strcpy(*(model_loader->material_names.push()), "skysphere");
#endif
  }

  // We've created all internal entities, so we will mark the next position
  // in the EntitySet, to know that that position is where the non-internal
  // entities start.
  state->entity_set.first_non_internal_handle = state->entity_set.next_handle;

  Memory::destroy_memory_pool(&temp_memory_pool);
}


void World::destroy_model_loaders(State *state) {
  state->model_loaders.clear();
}


void World::destroy_non_internal_materials(State *state) {
  for (
    uint32 idx = state->first_non_internal_material_idx;
    idx < state->materials.length;
    idx++
  ) {
    Materials::destroy_material(state->materials[idx]);
  }

  state->materials.delete_elements_after_index(
    state->first_non_internal_material_idx
  );
}


void World::destroy_non_internal_entities(State *state) {
  for (
    uint32 idx = state->entity_set.first_non_internal_handle;
    idx < state->entity_set.entities.length;
    idx++
  ) {
    Models::destroy_drawable_component(
      state->drawable_component_set.components[idx]
    );
  }

  state->entity_set.next_handle = state->entity_set.first_non_internal_handle;
  state->entity_set.entities.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->entity_loader_set.loaders.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->light_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->spatial_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->drawable_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->behavior_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->animation_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
}


void World::destroy_scene(State *state) {
  // If the current scene has not finished loading, we can neither
  // unload it nor load a new one.
  if (!state->is_world_loaded) {
    log_info(
      "Cannot load or unload scene while loading is already in progress."
    );
    return;
  }

  // TODO: Also reclaim texture names from TextureNamePool, otherwise we'll
  // end up overflowing.
  destroy_model_loaders(state);
  destroy_non_internal_materials(state);
  destroy_non_internal_entities(state);
}


bool32 World::load_scene(
  const char *scene_name,
  State *state
) {
  char log_text[512] = {0};

  // If the current scene has not finished loading, we can neither
  // unload it nor load a new one.
  if (!state->is_world_loaded) {
    Gui::console_print(
      &state->gui_state,
      "Cannot load or unload scene while loading is already in progress."
    );
    return false;
  }

  char scene_path[MAX_PATH] = {0};
  strcpy(scene_path, SCENE_DIR);
  strcat(scene_path, scene_name);
  strcat(scene_path, SCENE_EXTENSION);
  strcat(scene_path, "\0");

  strcpy(log_text, "Loading scene: ");
  strcat(log_text, scene_path);
  Gui::console_print(&state->gui_state, log_text);

  // Get some memory for everything we need
  MemoryPool temp_memory_pool = {};

  // Get EntityTemplates
  StackArray<EntityTemplate, 128> entity_templates;
  uint32 n_entities = 0;
  bool32 could_load_file = PeonyFileParser::parse_scene_file(
    scene_path, &entity_templates, &n_entities
  );

  if (!could_load_file) {
    strcpy(log_text, "Could not load scene: ");
    strcat(log_text, scene_path);
    Gui::console_print(&state->gui_state, log_text);
    return false;
  }

  // Destroy everything after we've confirmed we could load the scene.
  destroy_scene(state);

  strcpy(state->current_scene_name, scene_name);

  // Get only the unique used materials
  StackArray<char[MAX_TOKEN_LENGTH], MAX_N_MATERIALS> used_materials;
  for_each (entity_template, entity_templates) {
    for_each (material_name, entity_template->material_names) {
      bool32 does_material_already_exist = false;
      for_each (used_material, used_materials) {
        if (Str::eq(*material_name, *used_material)) {
          does_material_already_exist = true;
          break;
        }
      }
      if (!does_material_already_exist) {
        strcpy(*(used_materials.push()), *material_name);
      }
    }
  }

  // Create materials
  MaterialTemplate material_template;
  for_each (used_material, used_materials) {
    material_template = {};
    char path[MAX_PATH];
    PeonyFileParser::get_material_path(path, *used_material);
    PeonyFileParser::parse_material_file(path, &material_template);
    Materials::create_material_from_template(
      state->materials.push(),
      &material_template,
      &state->builtin_textures,
      &temp_memory_pool
    );
  }

  // Create entity, ModelLoader, EntityLoader
  for_range (0, n_entities) {
    EntityTemplate *entity_template = entity_templates[idx];
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, entity_template->entity_debug_name
    );

    // NOTE: We only want to make a ModelLoader from this EntityTemplate
    // if we haven't already encountered this model is a previous
    // EntityTemplate. If two entities have the same
    // `model_path_or_builtin_model_name`, we just make one model and use it
    // in both.
    ModelLoader *found_model_loader = state->model_loaders.find(
      [entity_template](ModelLoader *candidate_model_loader) -> bool32 {
        return Str::eq(
          entity_template->model_path_or_builtin_model_name,
          candidate_model_loader->model_path_or_builtin_model_name
        );
      }
    );
    if (found_model_loader) {
      log_info(
        "Skipping already-loaded model %s",
        entity_template->model_path_or_builtin_model_name
      );
    }
    if (!found_model_loader) {
      create_model_loader_from_entity_template(
        entity_template,
        entity->handle,
        &state->model_loaders,
        state
      );
    }
    create_entity_loader_from_entity_template(
      entity_template,
      entity->handle,
      &state->entity_loader_set,
      state
    );
  }

  // Clean up
  Memory::destroy_memory_pool(&temp_memory_pool);

  return true;
}


void World::init(State *state) {
  create_internal_materials(state);
  create_internal_entities(state);
}


bool32 World::check_all_entities_loaded(State *state) {
  bool are_all_done_loading = true;

  for_each (material, state->materials) {
    bool is_done_loading = Materials::prepare_material_and_check_if_done(
      material,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue
    );
    if (!is_done_loading) {
      are_all_done_loading = false;
    }
  }

  uint32 new_n_valid_model_loaders = 0;
  for_each (model_loader, state->model_loaders) {
    if (!Entities::is_model_loader_valid(model_loader)) {
      continue;
    }
    new_n_valid_model_loaders++;
    bool is_done_loading = Models::prepare_model_loader_and_check_if_done(
      model_loader,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue,
      &state->bone_matrix_pool
    );
    if (!is_done_loading) {
      are_all_done_loading = false;
    }
  }
  state->n_valid_model_loaders = new_n_valid_model_loaders;

  uint32 new_n_valid_entity_loaders = 0;
  for_each (entity_loader, state->entity_loader_set.loaders) {
    if (!Entities::is_entity_loader_valid(entity_loader)) {
      continue;
    }
    new_n_valid_entity_loaders++;

    ModelLoader *model_loader = state->model_loaders.find(
      [entity_loader](ModelLoader *candidate_model_loader) -> bool32 {
        return Str::eq(
          entity_loader->model_path_or_builtin_model_name,
          candidate_model_loader->model_path_or_builtin_model_name
        );
      }
    );
    if (!model_loader) {
      log_fatal(
        "Encountered an EntityLoader for which we cannot find the ModelLoader."
      );
    }

    bool is_done_loading = Models::prepare_entity_loader_and_check_if_done(
      entity_loader,
      &state->entity_set,
      model_loader,
      &state->drawable_component_set,
      &state->spatial_component_set,
      &state->light_component_set,
      &state->behavior_component_set,
      &state->animation_component_set,
      &state->physics_component_set
    );

    // NOTE: If a certain EntityLoader is complete, it's done everything it
    // needed to and we don't need it anymore.
    if (is_done_loading) {
      // TODO: We need to do this in a better way. We should somehow let the
      // Array know when we delete one of these. Even though it's sparse,
      // it should have length 0 if we know there's nothing in it. That way
      // we don't have to iterate over it over and over.
      memset(entity_loader, 0, sizeof(EntityLoader));
    }

    if (!is_done_loading) {
      are_all_done_loading = false;
    }
  }
  state->n_valid_entity_loaders = new_n_valid_entity_loaders;

  return are_all_done_loading;
}


void World::update(State *state) {
  if (state->is_world_loaded && !state->was_world_ever_loaded) {
    World::load_scene(DEFAULT_SCENE, state);
    state->was_world_ever_loaded = true;
  }

  Cameras::update_matrices(
    state->camera_active,
    state->window_info.width,
    state->window_info.height
  );
  state->is_world_loaded = check_all_entities_loaded(state);

  EntitySets::update_light_components(
    &state->light_component_set,
    &state->spatial_component_set,
    state->t,
    state->camera_active->position,
    state->dir_light_angle
  );

  EntitySets::update_behavior_components(
    state,
    &state->behavior_component_set,
    &state->spatial_component_set,
    state->t
  );

  EntitySets::update_animation_components(
    &state->animation_component_set,
    &state->spatial_component_set,
    state->t,
    &state->bone_matrix_pool
  );

  EntitySets::update_physics_components(
    &state->physics_component_set,
    &state->spatial_component_set
  );
}
