namespace engine {
  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  void destroy_model_loaders(State *state) {
    state->model_loaders.clear();
  }


  void destroy_non_internal_materials(State *state) {
    for (
      uint32 idx = state->first_non_internal_material_idx;
      idx < state->materials.length;
      idx++
    ) {
      materials::destroy_material(state->materials[idx]);
    }

    state->materials.delete_elements_after_index(
      state->first_non_internal_material_idx
    );
  }


  void destroy_non_internal_entities(State *state) {
    for (
      uint32 idx = state->entity_set.first_non_internal_handle;
      idx < state->entity_set.entities.length;
      idx++
    ) {
      models::destroy_drawable_component(
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


  void destroy_scene(State *state) {
    // If the current scene has not finished loading, we can neither
    // unload it nor load a new one.
    if (!state->is_world_loaded) {
      logs::info(
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


  bool32 load_scene(
    const char *scene_name,
    State *state
  ) {
    // If the current scene has not finished loading, we can neither
    // unload it nor load a new one.
    if (!state->is_world_loaded) {
      gui::log("Cannot load or unload scene while loading is already in progress.");
      return false;
    }

    char scene_path[MAX_PATH] = {0};
    strcpy(scene_path, SCENE_DIR);
    strcat(scene_path, scene_name);
    strcat(scene_path, SCENE_EXTENSION);
    strcat(scene_path, "\0");

    gui::log("Loading scene: %s", scene_path);

    // Get some memory for everything we need
    MemoryPool temp_memory_pool = {};

    // Get EntityTemplates
    StackArray<EntityTemplate, 128> entity_templates;
    uint32 n_entities = 0;
    bool32 could_load_file = peonyparser::parse_scene_file(
      scene_path, &entity_templates, &n_entities
    );

    if (!could_load_file) {
      gui::log("Could not load scene: %s", scene_path);
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
          if (str::eq(*material_name, *used_material)) {
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
      peonyparser::get_material_path(path, *used_material);
      peonyparser::parse_material_file(path, &material_template);
      peonyparser::create_material_from_template(
        state->materials.push(),
        &material_template,
        &state->builtin_textures,
        &temp_memory_pool
      );
    }

    // Create entity, ModelLoader, EntityLoader
    for_range (0, n_entities) {
      EntityTemplate *entity_template = entity_templates[idx];
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, entity_template->entity_debug_name
      );

      // NOTE: We only want to make a ModelLoader from this EntityTemplate
      // if we haven't already encountered this model is a previous
      // EntityTemplate. If two entities have the same
      // `model_path_or_builtin_model_name`, we just make one model and use it
      // in both.
      ModelLoader *found_model_loader = state->model_loaders.find(
        [entity_template](ModelLoader *candidate_model_loader) -> bool32 {
          return str::eq(
            entity_template->model_path_or_builtin_model_name,
            candidate_model_loader->model_path_or_builtin_model_name
          );
        }
      );
      if (found_model_loader) {
        logs::info(
          "Skipping already-loaded model %s",
          entity_template->model_path_or_builtin_model_name
        );
      }
      if (!found_model_loader) {
        peonyparser::create_model_loader_from_entity_template(
          entity_template,
          entity->handle,
          &state->model_loaders
        );
      }
      peonyparser::create_entity_loader_from_entity_template(
        entity_template,
        entity->handle,
        &state->entity_loader_set
      );
    }

    // Clean up
    memory::destroy_memory_pool(&temp_memory_pool);

    return true;
  }


  void handle_console_command(State *state) {
    gui::log("%s%s", gui::CONSOLE_SYMBOL, state->input_state.text_input);

    char command[input::MAX_TEXT_INPUT_COMMAND_LENGTH] = {0};
    char arguments[input::MAX_TEXT_INPUT_ARGUMENTS_LENGTH] = {0};

    str::split_on_first_occurrence(
      state->input_state.text_input,
      command, input::MAX_TEXT_INPUT_COMMAND_LENGTH,
      arguments, input::MAX_TEXT_INPUT_ARGUMENTS_LENGTH,
      ' '
    );

    if (str::eq(command, "help")) {
      gui::log(
        "Some useful commands\n"
        "--------------------\n"
        "loadscene <scene_name>: Load a scene\n"
        "renderdebug <internal_texture_name>: Display an internal texture. "
        "Use texture \"none\" to disable.\n"
        "help: show help"
      );
    } else if (str::eq(command, "loadscene")) {
      load_scene(arguments, state);
    } else if (str::eq(command, "renderdebug")) {
      state->renderdebug_displayed_texture_type = materials::texture_type_from_string(
        arguments
      );
    } else {
      gui::log("Unknown command: %s", command);
    }

    str::clear(state->input_state.text_input);
  }


  void update_light_position(State *state, real32 amount) {
    for_each (light_component, state->light_component_set.components) {
      if (light_component->type == LightType::directional) {
        state->dir_light_angle += amount;
        break;
      }
    }
  }


  void process_input(GLFWwindow *window, State *state) {
    if (input::is_key_now_down(&state->input_state, GLFW_KEY_GRAVE_ACCENT)) {
      if (state->game_console.is_enabled) {
        state->game_console.is_enabled = false;
        input::disable_text_input(&state->input_state);
      } else {
        state->game_console.is_enabled = true;
        input::enable_text_input(&state->input_state);
      }
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_ENTER)) {
      handle_console_command(state);
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_BACKSPACE)) {
      input::do_text_input_backspace(&state->input_state);
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_ESCAPE)) {
      input::clear_text_input(&state->input_state);
    }

    if (state->input_state.is_text_input_enabled) {
      // If we're typing text in, don't run any of the following stuff.
      return;
    }

    // Continuous
    if (input::is_key_down(&state->input_state, GLFW_KEY_W)) {
      cameras::move_front_back(state->camera_active, 1, state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_S)) {
      cameras::move_front_back(state->camera_active, -1, state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_A)) {
      cameras::move_left_right(state->camera_active, -1, state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_D)) {
      cameras::move_left_right(state->camera_active, 1, state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_Z)) {
      update_light_position(state, 0.10f * (real32)state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_X)) {
      update_light_position(state, -0.10f * (real32)state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_SPACE)) {
      cameras::move_up_down(state->camera_active, 1, state->dt);
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_LEFT_CONTROL)) {
      cameras::move_up_down(state->camera_active, -1, state->dt);
    }

    // Transient
    if (input::is_key_now_down(&state->input_state, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, true);
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_C)) {
      state->is_cursor_enabled = !state->is_cursor_enabled;
      renderer::update_drawing_options(state, window);
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_R)) {
      load_scene(
        state->current_scene_name, state
      );
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_TAB)) {
      state->should_pause = !state->should_pause;
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_MINUS)) {
      state->timescale_diff -= 0.1f;
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_EQUAL)) {
      state->timescale_diff += 0.1f;
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_BACKSPACE)) {
      state->should_hide_ui = !state->should_hide_ui;
    }

    if (input::is_key_down(&state->input_state, GLFW_KEY_ENTER)) {
      state->should_manually_advance_to_next_frame = true;
    }

    if (input::is_key_now_down(&state->input_state, GLFW_KEY_0)) {
      destroy_scene(state);
      gui::set_heading(&state->gui_state, "Scene destroyed", 1.0f, 1.0f, 1.0f);
    }
  }


  bool32 check_all_entities_loaded(State *state) {
    bool are_all_done_loading = true;

    for_each (material, state->materials) {
      bool is_done_loading = materials::prepare_material_and_check_if_done(
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
      if (!is_model_loader_valid(model_loader)) {
        continue;
      }
      new_n_valid_model_loaders++;
      bool is_done_loading = models::prepare_model_loader_and_check_if_done(
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
      if (!is_entity_loader_valid(entity_loader)) {
        continue;
      }
      new_n_valid_entity_loaders++;

      ModelLoader *model_loader = state->model_loaders.find(
        [entity_loader](ModelLoader *candidate_model_loader) -> bool32 {
          return str::eq(
            entity_loader->model_path_or_builtin_model_name,
            candidate_model_loader->model_path_or_builtin_model_name
          );
        }
      );
      if (!model_loader) {
        logs::fatal(
          "Encountered an EntityLoader for which we cannot find the ModelLoader."
        );
      }

      bool is_done_loading = models::prepare_entity_loader_and_check_if_done(
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


  void update(State *state) {
    if (state->is_world_loaded && !state->was_world_ever_loaded) {
      load_scene(DEFAULT_SCENE, state);
      state->was_world_ever_loaded = true;
    }

    cameras::update_matrices(
      state->camera_active,
      state->window_info.width,
      state->window_info.height
    );
    state->is_world_loaded = check_all_entities_loaded(state);

    lights::update_light_components(
      &state->light_component_set,
      &state->spatial_component_set,
      state->t,
      state->camera_active->position,
      state->dir_light_angle
    );

    behavior::update_behavior_components(
      state,
      &state->behavior_component_set,
      &state->spatial_component_set,
      state->t
    );

    anim::update_animation_components(
      &state->animation_component_set,
      &state->spatial_component_set,
      state->t,
      &state->bone_matrix_pool
    );

    physics::update_physics_components(
      &state->physics_component_set,
      &state->spatial_component_set
    );
  }


  void create_internal_materials(State *state) {
    MemoryPool temp_memory_pool = {};

    // unknown
    {
      Material *material = materials::init_material(
        state->materials.push(), "unknown"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "unknown", ShaderType::standard,
        "base.vert", "unknown.frag", ""
      );
    }

    // lighting
    {
      Material *material = materials::init_material(
        state->materials.push(), "lighting"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "lighting", ShaderType::standard,
        "screenquad.vert", "lighting.frag", ""
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_position_texture, "g_position_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
      );
    }

#if USE_BLOOM
    // preblur
    {
      Material *material = materials::init_material(
        state->materials.push(), "preblur"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "blur", ShaderType::standard,
        "screenquad.vert", "blur.frag", ""
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.l_bright_color_texture, "source_texture"
      );
    }

    // blur1
    {
      Material *material = materials::init_material(
        state->materials.push(), "blur1"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "blur", ShaderType::standard,
        "screenquad.vert", "blur.frag", ""
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.blur2_texture, "source_texture"
      );
    }

    // blur2
    {
      Material *material = materials::init_material(
        state->materials.push(), "blur2"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "blur", ShaderType::standard,
        "screenquad.vert", "blur.frag", ""
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.blur1_texture, "source_texture"
      );
    }
#endif

    // postprocessing
    {
      Material *material = materials::init_material(
        state->materials.push(), "postprocessing"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "postprocessing", ShaderType::standard,
        "screenquad.vert", "postprocessing.frag", ""
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.l_color_texture, "l_color_texture"
      );
#if USE_BLOOM
      materials::add_texture_to_material(
        material, *state->builtin_textures.blur2_texture, "bloom_texture"
      );
#endif
      // Uncomment to use fog.
      /* materials::add_texture_to-material( */
      /*   material, *state->l_depth_texture, "l_depth_texture" */
      /* ); */
    }

    // renderdebug
    {
      Material *material = materials::init_material(
        state->materials.push(), "renderdebug"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "renderdebug", ShaderType::standard,
        "screenquad.vert", "renderdebug.frag", ""
      );

      materials::add_texture_to_material(
        material, *state->builtin_textures.g_position_texture, "g_position_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
      );

      materials::add_texture_to_material(
        material, *state->builtin_textures.g_position_texture, "l_color_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.l_color_texture, "l_bright_color_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.l_color_texture, "l_depth_texture"
      );

      materials::add_texture_to_material(
        material, *state->builtin_textures.l_color_texture, "blur1_texture"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.l_color_texture, "blur2_texture"
      );

      materials::add_texture_to_material(
        material, *state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
      );
      materials::add_texture_to_material(
        material, *state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
      );
    }

    // skysphere
    {
      Material *material = materials::init_material(
        state->materials.push(), "skysphere"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "skysphere", ShaderType::standard,
        "skysphere.vert", "skysphere.frag", ""
      );
    }


    // We've created all internal materials, so we will mark the next position
    // in the array of materials, so we know where non-internal materials start.
    state->first_non_internal_material_idx = state->materials.length;

    memory::destroy_memory_pool(&temp_memory_pool);
  }


  void create_internal_entities(State *state) {
    MemoryPool temp_memory_pool = {};

    shaders::init_shader_asset(
      &state->standard_depth_shader_asset,
      &temp_memory_pool,
      "standard_depth", ShaderType::depth,
      "standard_depth.vert", "standard_depth.frag",
      "standard_depth.geom"
    );

    // Lighting screenquad
    {
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "screenquad_lighting"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader,
        ModelSource::data,
        "screenquad_lighting"
      );
      models::init_entity_loader(
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
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "screenquad_preblur"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader
        entity_loader,
        ModelSource::data,
        "screenquad_preblur"
      );
      models::init_entity_loader(
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
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "screenquad_blur1"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader,
        ModelSource::data,
        "screenquad_blur1"
      );
      models::init_entity_loader(
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
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "screenquad_blur2"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader,
        ModelSource::data,
        "screenquad_blur2",
      );
      models::init_entity_loader(
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
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "screenquad_postprocessing"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader,
        ModelSource::data,
        "screenquad_postprocessing"
      );
      models::init_entity_loader(
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
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "screenquad_renderdebug"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader,
        ModelSource::data,
        "screenquad_renderdebug"
      );
      models::init_entity_loader(
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
      Entity *entity = entities::add_entity_to_set(
        &state->entity_set, "skysphere"
      );
      ModelLoader *model_loader = state->model_loaders.push();
      EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(
        model_loader,
        ModelSource::data,
        "skysphere"
      );
      models::init_entity_loader(
        entity_loader,
        "skysphere",
        "skysphere",
        RenderPass::forward_skybox,
        entity->handle
      );
      entity_loader->spatial_component = {
        .entity_handle = entity->handle,
        .position = v3(0.0f),
        .rotation = glm::angleAxis(radians(0.0f), v3(1.0f, 0.0f, 0.0f)),
        .scale = v3(75.0f),
      };
      strcpy(*(model_loader->material_names.push()), "skysphere");
#endif
    }

    // We've created all internal entities, so we will mark the next position
    // in the EntitySet, to know that that position is where the non-internal
    // entities start.
    state->entity_set.first_non_internal_handle = state->entity_set.next_handle;

    memory::destroy_memory_pool(&temp_memory_pool);
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  void run_main_loop(State *state) {
    chrono::steady_clock::time_point second_start = chrono::steady_clock::now();
    chrono::steady_clock::time_point frame_start = chrono::steady_clock::now();
    chrono::steady_clock::time_point last_frame_start = chrono::steady_clock::now();
    // 1/165 seconds (for 165 fps)
    chrono::nanoseconds frame_duration = chrono::nanoseconds(6060606);
    chrono::steady_clock::time_point time_frame_should_end;
    uint32 n_frames_this_second = 0;
    uint32 n_frames_since_start = 0;

    while (!state->should_stop) {
      glfwPollEvents();
      process_input(state->window_info.window, state);

      if (
        !state->is_manual_frame_advance_enabled ||
        state->should_manually_advance_to_next_frame
      ) {
        n_frames_since_start++;
        last_frame_start = frame_start;
        frame_start = chrono::steady_clock::now();
        time_frame_should_end = frame_start + frame_duration;

        // If we should pause, stop time-based events.
        if (!state->should_pause) {
          state->dt = util::get_us_from_duration(frame_start - last_frame_start);
          if (state->timescale_diff != 0.0f) {
            state->dt *= 1.0f + state->timescale_diff;
          }

          state->perf_counters.dt_hist[state->perf_counters.dt_hist_idx] = state->dt;
          state->perf_counters.dt_hist_idx++;
          if (state->perf_counters.dt_hist_idx >= DT_HIST_LENGTH) {
            state->perf_counters.dt_hist_idx = 0;
          }
          real64 dt_hist_sum = 0.0f;
          for (uint32 idx = 0; idx < DT_HIST_LENGTH; idx++) {
            dt_hist_sum += state->perf_counters.dt_hist[idx];
          }
          state->perf_counters.dt_average = dt_hist_sum / DT_HIST_LENGTH;

          state->t += state->dt;
        }

        // Count FPS.
        n_frames_this_second++;
        chrono::duration<real64> time_since_second_start = frame_start - second_start;
        if (time_since_second_start >= chrono::seconds(1)) {
          second_start = frame_start;
          state->perf_counters.last_fps = n_frames_this_second;
          n_frames_this_second = 0;
          if (state->should_hide_ui) {
            logs::info("%.2f ms", state->perf_counters.dt_average * 1000.0f);
          }
        }

        // NOTE: Don't render on the very first frame. This avoids flashing that happens in
        // fullscreen. There is a better way to handle this, but whatever, figure it out later.
        if (n_frames_since_start > 1) {
          update(state);
          renderer::render(state);
        }
        if (state->is_manual_frame_advance_enabled) {
          state->should_manually_advance_to_next_frame = false;
        }
        input::reset_n_mouse_button_state_changes_this_frame(&state->input_state);
        input::reset_n_key_state_changes_this_frame(&state->input_state);

        if (state->should_limit_fps) {
          std::this_thread::sleep_until(time_frame_should_end);
        }
      }

      START_TIMER(swap_buffers);
      glfwSwapBuffers(state->window_info.window);
      END_TIMER_MIN(swap_buffers, 16);

      if (glfwWindowShouldClose(state->window_info.window)) {
        state->should_stop = true;
      }
    }
  }


  State* init_state(
    State *state,
    MemoryPool *asset_memory_pool,
    WindowInfo window_info
  ) {
    state->window_info = window_info;

    state->background_color = v4(0.81f, 0.93f, 1.00f, 1.0f);

    state->materials = Array<Material>(
      asset_memory_pool, MAX_N_MATERIALS, "materials"
    );
    state->model_loaders = Array<ModelLoader>(
      asset_memory_pool, MAX_N_MODELS, "model_loaders"
    );

    state->task_queue = Queue<Task>(asset_memory_pool, 128, "task_queue");

    state->builtin_textures = {
      .shadowmap_3d_width = min(state->window_info.width, (uint32)2000),
      .shadowmap_3d_height = min(state->window_info.width, (uint32)2000),
      .shadowmap_2d_width = 2560 * 2,
      .shadowmap_2d_height = 1440 * 2,
      .shadowmap_near_clip_dist = 0.05f,
      .shadowmap_far_clip_dist = 200.0f,
    };

    state->dir_light_angle = radians(55.0f);

    state->entity_loader_set = {
      .loaders = Array<EntityLoader>(
        asset_memory_pool, MAX_N_ENTITIES, "entity_loaders", true, 1
      )
    };
    state->entity_set = {
      .entities = Array<Entity>(
        asset_memory_pool, MAX_N_ENTITIES, "entities", true, 1
      )
    };
    state->drawable_component_set = {
      .components = Array<DrawableComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "drawable_components", true, 1
      )
    };
    state->light_component_set = {
      .components = Array<LightComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "light_components", true, 1
      )
    };
    state->spatial_component_set = {
      .components = Array<SpatialComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "spatial_components", true, 1
      )
    };
    state->behavior_component_set = {
      .components = Array<BehaviorComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "behavior_components", true, 1
      )
    };
    state->animation_component_set = {
      .components = Array<AnimationComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "animation_components", true, 1
      )
    };
    state->physics_component_set = {
      .components = Array<PhysicsComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "physics_components", true, 1
      )
    };
    state->bone_matrix_pool.bone_matrices = Array<m4>(
      asset_memory_pool,
      MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
      "bone_matrices",
      true
    );
    state->bone_matrix_pool.times = Array<real64>(
      asset_memory_pool,
      MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
      "bone_matrix_times",
      true
    );

    gui::g_console = &state->game_console;
    debugdraw::g_dds = &state->debug_draw_state;

    return state;
  }


  void init(State *state) {
    create_internal_materials(state);
    create_internal_entities(state);
  }
}
