#include <chrono>
namespace chrono = std::chrono;
#include <thread>
#include "../src_external/pstr.h"
#include "util.hpp"
#include "engine.hpp"
#include "logs.hpp"
#include "debug.hpp"
#include "peony_parser.hpp"
#include "peony_parser_utils.hpp"
#include "models.hpp"
#include "internals.hpp"
#include "renderer.hpp"
#include "intrinsics.hpp"


real64 *engine::g_t = nullptr;
real64 *engine::g_dt = nullptr;


namespace engine {
  pny_internal void destroy_model_loaders(EngineState *engine_state) {
    engine_state->model_loaders.clear();
  }


  pny_internal void destroy_non_internal_materials(
    EngineState *engine_state,
    MaterialsState *materials_state
  ) {
    for (
      uint32 idx = engine_state->first_non_internal_material_idx;
      idx < materials_state->materials.length;
      idx++
    ) {
      materials::destroy_material(materials_state->materials[idx]);
    }

    materials_state->materials.delete_elements_after_index(
      engine_state->first_non_internal_material_idx
    );
  }


  pny_internal void destroy_non_internal_entities(EngineState *engine_state) {
    for (
      uint32 idx = engine_state->entity_set.first_non_internal_handle;
      idx < engine_state->entity_set.entities.length;
      idx++
    ) {
      models::destroy_drawable_component(
        engine_state->drawable_component_set.components[idx]
      );
    }

    engine_state->entity_set.next_handle =
      engine_state->entity_set.first_non_internal_handle;
    engine_state->entity_set.entities.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
    engine_state->entity_loader_set.loaders.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
    engine_state->light_component_set.components.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
    engine_state->spatial_component_set.components.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
    engine_state->drawable_component_set.components.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
    engine_state->behavior_component_set.components.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
    engine_state->animation_component_set.components.delete_elements_after_index(
      engine_state->entity_set.first_non_internal_handle
    );
  }


  pny_internal void destroy_scene(
    EngineState *engine_state,
    MaterialsState *materials_state
  ) {
    // If the current scene has not finished loading, we can neither
    // unload it nor load a new one.
    if (!engine_state->is_world_loaded) {
      logs::info(
        "Cannot load or unload scene while loading is already in progress."
      );
      return;
    }

    // TODO: Also reclaim texture names from TextureNamePool, otherwise we'll
    // end up overflowing.
    destroy_model_loaders(engine_state);
    destroy_non_internal_materials(engine_state, materials_state);
    destroy_non_internal_entities(engine_state);
  }


  pny_internal bool32 load_scene(
    const char *scene_name,
    EngineState *engine_state,
    RendererState *renderer_state,
    MaterialsState *materials_state
  ) {
    // If the current scene has not finished loading, we can neither
    // unload it nor load a new one.
    if (!engine_state->is_world_loaded) {
      gui::log("Cannot load or unload scene while loading is already in progress.");
      return false;
    }

    // Get some memory for everything we need
    MemoryPool temp_memory_pool = {};
    defer { memory::destroy_memory_pool(&temp_memory_pool); };

    // Load scene file
    char scene_path[MAX_PATH] = {};
    pstr_vcat(scene_path, MAX_PATH, SCENE_DIR, scene_name, SCENE_EXTENSION, NULL);
    gui::log("Loading scene: %s", scene_path);

    PeonyFile *scene_file = MEMORY_PUSH(&temp_memory_pool, PeonyFile, "scene_file");
    if (!peony_parser::parse_file(scene_file, scene_path)) {
      gui::log("Could not load scene: %s", scene_path);
      return false;
    }

    // Destroy our current scene after we've confirmed we could load the new scene.
    destroy_scene(engine_state, materials_state);
    pstr_copy(engine_state->current_scene_name, MAX_COMMON_NAME_LENGTH, scene_name);

    // Get only the unique used materials
    Array<char[MAX_COMMON_NAME_LENGTH]> used_materials(
      &temp_memory_pool, MAX_N_MATERIALS, "used_materials"
    );
    peony_parser_utils::get_unique_string_values_for_prop_name(
      scene_file, &used_materials, "materials"
    );

    // Create Materials
    PeonyFile *material_file = MEMORY_PUSH(&temp_memory_pool, PeonyFile, "material_file");
    each (used_material, used_materials) {
      memset(material_file, 0, sizeof(PeonyFile));
      char material_file_path[MAX_PATH] = {};
      pstr_vcat(
        material_file_path, MAX_PATH,
        MATERIAL_FILE_DIRECTORY, *used_material, MATERIAL_FILE_EXTENSION, nullptr
      );
      if (!peony_parser::parse_file(material_file, material_file_path)) {
        gui::log("Could not load material: %s", material_file_path);
        break;
      }
      assert(material_file->n_entries > 0);
      peony_parser_utils::create_material_from_peony_file_entry(
        materials_state->materials.push(),
        &material_file->entries[0],
        &renderer_state->builtin_textures,
        &temp_memory_pool
      );
    }

    range (0, scene_file->n_entries) {
      PeonyFileEntry *entry = &scene_file->entries[idx];

      // Create Entity
      Entity *entity = entities::add_entity_to_set(
        &engine_state->entity_set, entry->name
      );

      // Create ModelLoader
      char const *model_path = peony_parser_utils::get_string(
        peony_parser_utils::find_prop(entry, "model_path")
      );
      // NOTE: We only want to make a ModelLoader from this PeonyFileEntry if we haven't
      // already encountered this model in a previous entry. If two entities
      // have the same `model_path`, we just make one model and use it in both.
      ModelLoader *found_model_loader = engine_state->model_loaders.find(
        [model_path](ModelLoader *candidate_model_loader) -> bool32 {
          return pstr_eq(
            model_path,
            candidate_model_loader->model_path
          );
        }
      );
      if (found_model_loader) {
        logs::info("Skipping already-loaded model %s", model_path);
      } else {
        peony_parser_utils::create_model_loader_from_peony_file_entry(
          entry,
          entity->handle,
          engine_state->model_loaders.push()
        );
      }

      // Create EntityLoader
      peony_parser_utils::create_entity_loader_from_peony_file_entry(
        entry,
        entity->handle,
        engine_state->entity_loader_set.loaders[entity->handle]
      );
    }

    return true;
  }


  pny_internal void handle_console_command(
    EngineState *engine_state,
    RendererState *renderer_state,
    MaterialsState *materials_state,
    InputState *input_state
  ) {
    gui::log("%s%s", gui::CONSOLE_SYMBOL, input_state->text_input);

    char command[input::MAX_TEXT_INPUT_COMMAND_LENGTH] = {0};
    char arguments[input::MAX_TEXT_INPUT_ARGUMENTS_LENGTH] = {0};

    pstr_split_on_first_occurrence(
      input_state->text_input,
      command, input::MAX_TEXT_INPUT_COMMAND_LENGTH,
      arguments, input::MAX_TEXT_INPUT_ARGUMENTS_LENGTH,
      ' '
    );

    if (pstr_eq(command, "help")) {
      gui::log(
        "Some useful commands\n"
        "--------------------\n"
        "loadscene <scene_name>: Load a scene\n"
        "renderdebug <internal_texture_name>: Display an internal texture. "
        "Use texture \"none\" to disable.\n"
        "help: show help"
      );
    } else if (pstr_eq(command, "loadscene")) {
      load_scene(arguments, engine_state, renderer_state, materials_state);
    } else if (pstr_eq(command, "renderdebug")) {
      renderer_state->renderdebug_displayed_texture_type =
        materials::texture_type_from_string(arguments);
    } else {
      gui::log("Unknown command: %s", command);
    }

    pstr_clear(input_state->text_input);
  }


  pny_internal void update_light_position(
    EngineState *engine_state,
    LightsState *lights_state,
    real32 amount
  ) {
    each (light_component, engine_state->light_component_set.components) {
      if (light_component->type == LightType::directional) {
        lights_state->dir_light_angle += amount;
        break;
      }
    }
  }


  pny_internal void process_input(
    GLFWwindow *window,
    EngineState *engine_state,
    RendererState *renderer_state,
    MaterialsState *materials_state,
    InputState *input_state,
    GuiState *gui_state,
    CamerasState *cameras_state,
    LightsState *lights_state
  ) {
    if (input::is_key_now_down(input_state, GLFW_KEY_GRAVE_ACCENT)) {
      if (gui_state->game_console.is_enabled) {
        gui_state->game_console.is_enabled = false;
        input::disable_text_input(input_state);
      } else {
        gui_state->game_console.is_enabled = true;
        input::enable_text_input(input_state);
      }
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_ENTER)) {
      handle_console_command(
        engine_state, renderer_state, materials_state, input_state
      );
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_BACKSPACE)) {
      input::do_text_input_backspace(input_state);
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_ESCAPE)) {
      input::clear_text_input(input_state);
    }

    if (input_state->is_text_input_enabled) {
      // If we're typing text in, don't run any of the following stuff.
      return;
    }

    // Continuous
    if (input::is_key_down(input_state, GLFW_KEY_W)) {
      cameras::move_front_back(cameras_state->camera_active, 1, *engine::g_dt);
    }

    if (input::is_key_down(input_state, GLFW_KEY_S)) {
      cameras::move_front_back(cameras_state->camera_active, -1, *engine::g_dt);
    }

    if (input::is_key_down(input_state, GLFW_KEY_A)) {
      cameras::move_left_right(cameras_state->camera_active, -1, *engine::g_dt);
    }

    if (input::is_key_down(input_state, GLFW_KEY_D)) {
      cameras::move_left_right(cameras_state->camera_active, 1, *engine::g_dt);
    }

    if (input::is_key_down(input_state, GLFW_KEY_Z)) {
      update_light_position(engine_state, lights_state, 0.10f * (real32)(*engine::g_dt));
    }

    if (input::is_key_down(input_state, GLFW_KEY_X)) {
      update_light_position(engine_state, lights_state, -0.10f * (real32)(*engine::g_dt));
    }

    if (input::is_key_down(input_state, GLFW_KEY_SPACE)) {
      cameras::move_up_down(cameras_state->camera_active, 1, *engine::g_dt);
    }

    if (input::is_key_down(input_state, GLFW_KEY_LEFT_CONTROL)) {
      cameras::move_up_down(cameras_state->camera_active, -1, *engine::g_dt);
    }

    // Transient
    if (input::is_key_now_down(input_state, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, true);
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_C)) {
      renderer_state->is_cursor_enabled = !renderer_state->is_cursor_enabled;
      renderer::update_drawing_options(renderer_state, window);
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_R)) {
      load_scene(
        engine_state->current_scene_name,
        engine_state,
        renderer_state,
        materials_state
      );
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_TAB)) {
      engine_state->should_pause = !engine_state->should_pause;
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_MINUS)) {
      engine_state->timescale_diff -= 0.1f;
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_EQUAL)) {
      engine_state->timescale_diff += 0.1f;
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_BACKSPACE)) {
      renderer_state->should_hide_ui = !renderer_state->should_hide_ui;
    }

    if (input::is_key_down(input_state, GLFW_KEY_ENTER)) {
      engine_state->should_manually_advance_to_next_frame = true;
    }

    if (input::is_key_now_down(input_state, GLFW_KEY_0)) {
      destroy_scene(engine_state, materials_state);
      gui::set_heading(gui_state, "Scene destroyed", 1.0f, 1.0f, 1.0f);
    }
  }


  pny_internal bool32 check_all_entities_loaded(
    EngineState *engine_state,
    MaterialsState *materials_state,
    TasksState *tasks_state,
    AnimState *anim_state
  ) {
    bool are_all_done_loading = true;

    each (material, materials_state->materials) {
      bool is_done_loading = materials::prepare_material_and_check_if_done(
        material,
        &materials_state->persistent_pbo,
        &materials_state->texture_name_pool,
        &tasks_state->task_queue
      );
      if (!is_done_loading) {
        are_all_done_loading = false;
      }
    }

    uint32 new_n_valid_model_loaders = 0;
    each (model_loader, engine_state->model_loaders) {
      if (!is_model_loader_valid(model_loader)) {
        continue;
      }
      new_n_valid_model_loaders++;
      bool is_done_loading = models::prepare_model_loader_and_check_if_done(
        model_loader,
        &materials_state->persistent_pbo,
        &materials_state->texture_name_pool,
        &tasks_state->task_queue,
        &anim_state->bone_matrix_pool
      );
      if (!is_done_loading) {
        are_all_done_loading = false;
      }
    }
    engine_state->n_valid_model_loaders = new_n_valid_model_loaders;

    uint32 new_n_valid_entity_loaders = 0;
    each (entity_loader, engine_state->entity_loader_set.loaders) {
      if (!is_entity_loader_valid(entity_loader)) {
        continue;
      }
      new_n_valid_entity_loaders++;

      ModelLoader *model_loader = engine_state->model_loaders.find(
        [entity_loader](ModelLoader *candidate_model_loader) -> bool32 {
          return pstr_eq(entity_loader->model_path, candidate_model_loader->model_path);
        }
      );
      if (!model_loader) {
        logs::fatal(
          "Encountered an EntityLoader %d for which we cannot find the ModelLoader.",
          entity_loader->entity_handle
        );
      }

      bool is_done_loading = models::prepare_entity_loader_and_check_if_done(
        entity_loader,
        &engine_state->entity_set,
        model_loader,
        &engine_state->drawable_component_set,
        &engine_state->spatial_component_set,
        &engine_state->light_component_set,
        &engine_state->behavior_component_set,
        &engine_state->animation_component_set,
        &engine_state->physics_component_set
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
    engine_state->n_valid_entity_loaders = new_n_valid_entity_loaders;

    return are_all_done_loading;
  }


  pny_internal void update(
    EngineState *engine_state,
    RendererState *renderer_state,
    MaterialsState *materials_state,
    CamerasState *cameras_state,
    TasksState *tasks_state,
    AnimState *anim_state,
    LightsState *lights_state,
    WindowSize *window_size
  ) {
    if (engine_state->is_world_loaded && !engine_state->was_world_ever_loaded) {
      load_scene(DEFAULT_SCENE, engine_state, renderer_state, materials_state);
      engine_state->was_world_ever_loaded = true;
    }

    cameras::update_matrices(
      cameras_state->camera_active,
      window_size->width,
      window_size->height
    );

    engine_state->is_world_loaded = check_all_entities_loaded(
      engine_state,
      materials_state,
      tasks_state,
      anim_state
    );

    lights::update_light_components(
      lights_state,
      &engine_state->light_component_set,
      &engine_state->spatial_component_set,
      cameras_state->camera_active->position
    );

    #if 0
    behavior::update_behavior_components(
      state,
      &state->behavior_component_set,
      &state->spatial_component_set,
      state->t
    );
    #endif

    anim::update_animation_components(
      &engine_state->animation_component_set,
      &engine_state->spatial_component_set,
      &anim_state->bone_matrix_pool
    );

    physics::update_physics_components(
      &engine_state->physics_component_set,
      &engine_state->spatial_component_set
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


  pny_internal void update_dt_and_perf_counters(
    EngineState *engine_state,
    TimingInfo *timing
  ) {
    engine_state->dt = util::get_us_from_duration(
      timing->frame_start - timing->last_frame_start
    );
    if (engine_state->timescale_diff != 0.0f) {
      engine_state->dt *= max(1.0f + engine_state->timescale_diff, (real64)0.01f);
    }

    engine_state->perf_counters.dt_hist[engine_state->perf_counters.dt_hist_idx] =
      engine_state->dt;
    engine_state->perf_counters.dt_hist_idx++;
    if (engine_state->perf_counters.dt_hist_idx >= DT_HIST_LENGTH) {
      engine_state->perf_counters.dt_hist_idx = 0;
    }
    real64 dt_hist_sum = 0.0f;
    for (uint32 idx = 0; idx < DT_HIST_LENGTH; idx++) {
      dt_hist_sum += engine_state->perf_counters.dt_hist[idx];
    }
    engine_state->perf_counters.dt_average = dt_hist_sum / DT_HIST_LENGTH;

    engine_state->t += engine_state->dt;
  }
}


void engine::run_main_loop(
  EngineState *engine_state,
  RendererState *renderer_state,
  MaterialsState *materials_state,
  CamerasState *cameras_state,
  GuiState *gui_state,
  InputState *input_state,
  LightsState *lights_state,
  TasksState *tasks_state,
  AnimState *anim_state,
  GLFWwindow *window,
  WindowSize *window_size
) {
  TimingInfo timing = init_timing_info(165);

  while (!engine_state->should_stop) {
    glfwPollEvents();
    process_input(
      window,
      engine_state,
      renderer_state,
      materials_state,
      input_state,
      gui_state,
      cameras_state,
      lights_state
    );

    if (
      !engine_state->is_manual_frame_advance_enabled ||
      engine_state->should_manually_advance_to_next_frame
    ) {
      update_timing_info(&timing, &engine_state->perf_counters.last_fps);

      // If we should pause, stop time-based events.
      if (!engine_state->should_pause) {
        update_dt_and_perf_counters(engine_state, &timing);
      }

      // NOTE: Don't render on the very first frame. This avoids flashing that happens
      // in fullscreen. There is a better way to handle this, but whatever, figure it
      // out later.
      if (timing.n_frames_since_start > 1) {
        update(
          engine_state,
          renderer_state,
          materials_state,
          cameras_state,
          tasks_state,
          anim_state,
          lights_state,
          window_size
        );
        renderer::render(
          renderer_state,
          engine_state,
          materials_state,
          cameras_state,
          gui_state,
          input_state,
          window,
          window_size
        );
      }

      if (engine_state->is_manual_frame_advance_enabled) {
        engine_state->should_manually_advance_to_next_frame = false;
      }

      input::reset_n_mouse_button_state_changes_this_frame(input_state);
      input::reset_n_key_state_changes_this_frame(input_state);

      if (engine_state->should_limit_fps) {
        std::this_thread::sleep_until(timing.time_frame_should_end);
      }
    }


    if (glfwWindowShouldClose(window)) {
      engine_state->should_stop = true;
    }
  }
}


void engine::init(EngineState *engine_state, MemoryPool *asset_memory_pool) {
  engine_state->model_loaders = Array<ModelLoader>(
    asset_memory_pool, MAX_N_MODELS, "model_loaders"
  );
  engine_state->entity_loader_set = {
    .loaders = Array<EntityLoader>(
      asset_memory_pool, MAX_N_ENTITIES, "entity_loaders", true, 1
    )
  };
  engine_state->entity_set = {
    .entities = Array<Entity>(
      asset_memory_pool, MAX_N_ENTITIES, "entities", true, 1
    )
  };
  engine_state->drawable_component_set = {
    .components = Array<DrawableComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "drawable_components", true, 1
    )
  };
  engine_state->light_component_set = {
    .components = Array<LightComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "light_components", true, 1
    )
  };
  engine_state->spatial_component_set = {
    .components = Array<SpatialComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "spatial_components", true, 1
    )
  };
  engine_state->behavior_component_set = {
    .components = Array<BehaviorComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "behavior_components", true, 1
    )
  };
  engine_state->animation_component_set = {
    .components = Array<AnimationComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "animation_components", true, 1
    )
  };
  engine_state->physics_component_set = {
    .components = Array<PhysicsComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "physics_components", true, 1
    )
  };

  engine::g_t = &engine_state->t;
  engine::g_dt = &engine_state->dt;
}
