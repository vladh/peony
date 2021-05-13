void engine::handle_console_command(State *state) {
  logs::console("%s%s", GUI_CONSOLE_SYMBOL, state->input_state.text_input);

  char command[MAX_TEXT_INPUT_COMMAND_LENGTH] = {0};
  char arguments[MAX_TEXT_INPUT_ARGUMENTS_LENGTH] = {0};

  str::split_on_first_occurrence(
    state->input_state.text_input,
    command, MAX_TEXT_INPUT_COMMAND_LENGTH,
    arguments, MAX_TEXT_INPUT_ARGUMENTS_LENGTH,
    ' '
  );

  if (str::eq(command, "help")) {
    logs::console(
      "Some useful commands\n"
      "--------------------\n"
      "loadscene <scene_name>: Load a scene\n"
      "renderdebug <internal_texture_name>: Display an internal texture. "
      "Use texture \"none\" to disable.\n"
      "help: show help"
    );
  } else if (str::eq(command, "loadscene")) {
    world::load_scene(arguments, state);
  } else if (str::eq(command, "renderdebug")) {
    state->renderdebug_displayed_texture_type = materials::texture_type_from_string(
      arguments
    );
  } else {
    logs::console("Unknown command: %s", command);
  }

  str::clear(state->input_state.text_input);
}


void engine::process_input(GLFWwindow *window, State *state) {
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
    world::update_light_position(state, 0.10f * (real32)state->dt);
  }

  if (input::is_key_down(&state->input_state, GLFW_KEY_X)) {
    world::update_light_position(state, -0.10f * (real32)state->dt);
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
    world::load_scene(
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
    world::destroy_scene(state);
    renderer::set_heading(state, "Scene destroyed", 1.0f, 1.0f, 1.0f);
  }
}


void engine::run_main_loop(State *state) {
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
        g_t = state->t;
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
        world::update(state);
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


State* engine::init_state(
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

  g_console = &state->game_console;
  g_dds = &state->debug_draw_state;

  return state;
}
