#include <chrono>
namespace chrono = std::chrono;
#include "util.hpp"
#include "engine.hpp"
#include "logs.hpp"
#include "str.hpp"
#include "debug.hpp"
#include "peony_parser.hpp"
#include "models.hpp"
#include "internals.hpp"
#include "renderer.hpp"
#include "intrinsics.hpp"


namespace engine {
  pny_internal void destroy_model_loaders(State *state) {
    state->model_loaders.clear();
  }


  pny_internal void destroy_non_internal_materials(State *state) {
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


  pny_internal void destroy_non_internal_entities(State *state) {
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


  pny_internal void destroy_scene(State *state) {
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


  pny_internal bool32 load_scene(
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
    bool32 could_load_file = peony_parser::parse_scene_file(
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
    each (entity_template, entity_templates) {
      each (material_name, entity_template->material_names) {
        bool32 does_material_already_exist = false;
        each (used_material, used_materials) {
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
    each (used_material, used_materials) {
      material_template = {};
      char path[MAX_PATH];
      peony_parser::get_material_path(path, *used_material);
      peony_parser::parse_material_file(path, &material_template);
      peony_parser::create_material_from_template(
        state->materials.push(),
        &material_template,
        &state->builtin_textures,
        &temp_memory_pool
      );
    }

    // Create entity, ModelLoader, EntityLoader
    range (0, n_entities) {
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
        peony_parser::create_model_loader_from_entity_template(
          entity_template,
          entity->handle,
          &state->model_loaders
        );
      }
      peony_parser::create_entity_loader_from_entity_template(
        entity_template,
        entity->handle,
        &state->entity_loader_set
      );
    }

    // Clean up
    memory::destroy_memory_pool(&temp_memory_pool);

    return true;
  }


  pny_internal void handle_console_command(State *state) {
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


  pny_internal void update_light_position(State *state, real32 amount) {
    each (light_component, state->light_component_set.components) {
      if (light_component->type == LightType::directional) {
        state->dir_light_angle += amount;
        break;
      }
    }
  }


  pny_internal void process_input(GLFWwindow *window, State *state) {
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


  pny_internal bool32 check_all_entities_loaded(State *state) {
    bool are_all_done_loading = true;

    each (material, state->materials) {
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
    each (model_loader, state->model_loaders) {
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
    each (entity_loader, state->entity_loader_set.loaders) {
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


  pny_internal void update(State *state) {
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


  pny_internal TimingInfo init_timing_info(uint32 target_fps) {
    TimingInfo timing_info = {};
    timing_info.second_start = chrono::steady_clock::now();
    timing_info.frame_start = chrono::steady_clock::now();
    timing_info.last_frame_start = chrono::steady_clock::now();
    timing_info.frame_duration = chrono::nanoseconds(
      (uint32)((real64)1.0f / (real64)target_fps)
    );
    return timing_info;
  }


  pny_internal void update_timing_info(TimingInfo *timing, uint32 *last_fps) {
    timing->n_frames_since_start++;
    timing->last_frame_start = timing->frame_start;
    timing->frame_start = chrono::steady_clock::now();
    timing->time_frame_should_end = timing->frame_start + timing->frame_duration;

    timing->n_frames_this_second++;
    chrono::duration<real64> time_since_second_start =
      timing->frame_start - timing->second_start;
    if (time_since_second_start >= chrono::seconds(1)) {
      timing->second_start = timing->frame_start;
      *last_fps = timing->n_frames_this_second;
      timing->n_frames_this_second = 0;
    }
  }


  pny_internal void update_dt_and_perf_counters(State *state, TimingInfo *timing) {
    state->dt = util::get_us_from_duration(
      timing->frame_start - timing->last_frame_start
    );
    if (state->timescale_diff != 0.0f) {
      state->dt *= max(1.0f + state->timescale_diff, (real64)0.01f);
    }

    state->perf_counters.dt_hist[state->perf_counters.dt_hist_idx] = state->dt;
    state->perf_counters.dt_hist_idx++;
    if (state->perf_counters.dt_hist_idx >= state::DT_HIST_LENGTH) {
      state->perf_counters.dt_hist_idx = 0;
    }
    real64 dt_hist_sum = 0.0f;
    for (uint32 idx = 0; idx < state::DT_HIST_LENGTH; idx++) {
      dt_hist_sum += state->perf_counters.dt_hist[idx];
    }
    state->perf_counters.dt_average = dt_hist_sum / state::DT_HIST_LENGTH;

    state->t += state->dt;
  }
}


void engine::run_main_loop(State *state) {
  TimingInfo timing = init_timing_info(165);

  while (!state->should_stop) {
    glfwPollEvents();
    process_input(state->window_info.window, state);

    if (
      !state->is_manual_frame_advance_enabled ||
      state->should_manually_advance_to_next_frame
    ) {
      update_timing_info(&timing, &state->perf_counters.last_fps);

      // If we should pause, stop time-based events.
      if (!state->should_pause) { update_dt_and_perf_counters(state, &timing); }

      // NOTE: Don't render on the very first frame. This avoids flashing that happens
      // in fullscreen. There is a better way to handle this, but whatever, figure it
      // out later.
      if (timing.n_frames_since_start > 1) {
        update(state);
        renderer::render(state);
      }

      if (state->is_manual_frame_advance_enabled) {
        state->should_manually_advance_to_next_frame = false;
      }

      input::reset_n_mouse_button_state_changes_this_frame(&state->input_state);
      input::reset_n_key_state_changes_this_frame(&state->input_state);

      if (state->should_limit_fps) {
        std::this_thread::sleep_until(timing.time_frame_should_end);
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


void engine::init(State *state) {
  internals::create_internal_materials(state);
  internals::create_internal_entities(state);
}
