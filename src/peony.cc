#define _CRT_SECURE_NO_WARNINGS

#define USE_TIMERS true
#define USE_VLD false
#define USE_MEMORY_DEBUG_LOGS false
#define USE_MEMORYPOOL_ITEM_DEBUG false
#define USE_CACHELINE_SIZE_DISPLAY false
#define USE_FULLSCREEN true
#define USE_WINDOWED_FULLSCREEN true
#define USE_ANIMATION_DEBUG false
#define USE_SHADER_DEBUG false
#define USE_BLOOM false
#define TARGET_MONITOR 0

#include "peony.hpp"

global real64 g_t;
global GameConsole g_console;

#include "log.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "str.cpp"
#include "tasks.cpp"
#include "peony_file_parser.cpp"
#include "materials.cpp"
#include "fonts.cpp"
#include "shaders.cpp"
#include "camera.cpp"
#include "memory.cpp"
#include "input.cpp"
#include "physics.cpp"
#include "entities.cpp"
#include "behavior_functions.cpp"
#include "entity_sets.cpp"
#include "gui.cpp"
#include "debugdraw.cpp"
#include "models.cpp"
#include "world.cpp"
#include "state.cpp"
#include "renderer.cpp"


void handle_console_command(State *state) {
  console_log("%s%s", GUI_CONSOLE_SYMBOL, state->input_state.text_input);

  char command[MAX_TEXT_INPUT_COMMAND_LENGTH] = {0};
  char arguments[MAX_TEXT_INPUT_ARGUMENTS_LENGTH] = {0};

  Str::split_on_first_occurrence(
    state->input_state.text_input,
    command, MAX_TEXT_INPUT_COMMAND_LENGTH,
    arguments, MAX_TEXT_INPUT_ARGUMENTS_LENGTH,
    ' '
  );

  if (Str::eq(command, "help")) {
    console_log(
      "Some useful commands\n"
      "--------------------\n"
      "loadscene <scene_name>: Load a scene\n"
      "renderdebug <internal_texture_name>: Display an internal texture. "
      "Use texture \"none\" to disable.\n"
      "help: show help"
    );
  } else if (Str::eq(command, "loadscene")) {
    World::load_scene(arguments, state);
  } else if (Str::eq(command, "renderdebug")) {
    state->renderdebug_displayed_texture_type = Materials::texture_type_from_string(
      arguments
    );
  } else {
    console_log("Unknown command: %s", command);
  }

  Str::clear(state->input_state.text_input);
}


void process_input(GLFWwindow *window, State *state) {
  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_GRAVE_ACCENT)) {
    if (g_console.is_enabled) {
      g_console.is_enabled = false;
      Input::disable_text_input(&state->input_state);
    } else {
      g_console.is_enabled = true;
      Input::enable_text_input(&state->input_state);
    }
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_ENTER)) {
    handle_console_command(state);
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_BACKSPACE)) {
    Input::do_text_input_backspace(&state->input_state);
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_ESCAPE)) {
    Input::clear_text_input(&state->input_state);
  }

  if (state->input_state.is_text_input_enabled) {
    // If we're typing text in, don't run any of the following stuff.
    return;
  }

  // Continuous
  if (Input::is_key_down(&state->input_state, GLFW_KEY_W)) {
    Cameras::move_front_back(state->camera_active, 1, state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_S)) {
    Cameras::move_front_back(state->camera_active, -1, state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_A)) {
    Cameras::move_left_right(state->camera_active, -1, state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_D)) {
    Cameras::move_left_right(state->camera_active, 1, state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_Z)) {
    World::update_light_position(state, 0.10f * (real32)state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_X)) {
    World::update_light_position(state, -0.10f * (real32)state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_SPACE)) {
    Cameras::move_up_down(state->camera_active, 1, state->dt);
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_LEFT_CONTROL)) {
    Cameras::move_up_down(state->camera_active, -1, state->dt);
  }

  // Transient
  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_C)) {
    state->is_cursor_enabled = !state->is_cursor_enabled;
    Renderer::update_drawing_options(state, window);
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_R)) {
    World::load_scene(
      state->current_scene_name, state
    );
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_TAB)) {
    state->should_pause = !state->should_pause;
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_BACKSPACE)) {
    state->should_hide_ui = !state->should_hide_ui;
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_ENTER)) {
    state->should_manually_advance_to_next_frame = true;
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_0)) {
    World::destroy_scene(state);
    Renderer::set_heading(state, "Scene destroyed", 1.0f, 1.0f, 1.0f);
  }
}


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
        state->dt = Util::get_us_from_duration(frame_start - last_frame_start);

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
          log_info("%.2f ms", state->perf_counters.dt_average * 1000.0f);
        }
      }

      // NOTE: Don't render on the very first frame. This avoids flashing that happens in
      // fullscreen. There is a better way to handle this, but whatever, figure it out later.
      if (n_frames_since_start > 1) {
        World::update(state);
        Renderer::render(state);
      }
      if (state->is_manual_frame_advance_enabled) {
        state->should_manually_advance_to_next_frame = false;
      }
      Input::reset_n_mouse_button_state_changes_this_frame(&state->input_state);
      Input::reset_n_key_state_changes_this_frame(&state->input_state);

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


void check_environment() {
  // Check that an `enum class`'s default value == its first element == 0;
  BehaviorComponent test;
  assert(test.behavior == Behavior::none);
  assert(static_cast<int>(test.behavior) == 0);
}


int main() {
  check_environment();
  srand((uint32)time(NULL));

  MemoryPool state_memory_pool = {};
  state_memory_pool.size = sizeof(State);
  MemoryPool asset_memory_pool = {.size = Util::mb_to_b(1024)};

  WindowInfo window_info;
  Renderer::init_window(&window_info);
  if (!window_info.window) {
    return -1;
  }

  State *state = init_state(
    (State*)Memory::push(
      &state_memory_pool, sizeof(State), "state"
    ),
    &asset_memory_pool,
    window_info
  );

  std::mutex loading_thread_mutex;
  std::thread loading_thread_1 = std::thread(
    Tasks::run_loading_loop, &loading_thread_mutex, state, 1
  );
  std::thread loading_thread_2 = std::thread(
    Tasks::run_loading_loop, &loading_thread_mutex, state, 2
  );
  std::thread loading_thread_3 = std::thread(
    Tasks::run_loading_loop, &loading_thread_mutex, state, 3
  );
  std::thread loading_thread_4 = std::thread(
    Tasks::run_loading_loop, &loading_thread_mutex, state, 4
  );
  std::thread loading_thread_5 = std::thread(
    Tasks::run_loading_loop, &loading_thread_mutex, state, 5
  );

  Renderer::update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&asset_memory_pool, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  Materials::init_texture_name_pool(
    &state->texture_name_pool, &asset_memory_pool, 64, 4
  );
  Renderer::init_g_buffer(
    &asset_memory_pool, &state->builtin_textures, window_info.width, window_info.height
  );
  Renderer::init_l_buffer(
    &asset_memory_pool, &state->builtin_textures, window_info.width, window_info.height
  );
  Renderer::init_blur_buffers(
    &asset_memory_pool, &state->builtin_textures, window_info.width, window_info.height
  );
  Renderer::init_shadowmaps(
    &asset_memory_pool, &state->builtin_textures
  );
  Renderer::init_ubo(state);
  World::init(state);

  Materials::init_persistent_pbo(&state->persistent_pbo, 25, 2048, 2048, 4);

  Gui::init_gui_state(
    &state->gui_state,
    &asset_memory_pool,
    &state->input_state,
    state->window_info.width, state->window_info.height
  );
  console_log("Hello world!");

  DebugDraw::init_debug_draw_state(
    &state->debug_draw_state,
    &asset_memory_pool
  );

  Cameras::init_camera(
    &state->camera_main,
    CameraType::perspective,
    state->window_info.width,
    state->window_info.height
  );
  state->camera_active = &state->camera_main;

  Input::init_input_state(
    &state->input_state,
    state->window_info.window
  );

  run_main_loop(state);

  loading_thread_1.join();
  loading_thread_2.join();
  loading_thread_3.join();
  loading_thread_4.join();
  loading_thread_5.join();

  Renderer::destroy_window();
  Memory::destroy_memory_pool(&state_memory_pool);
  Memory::destroy_memory_pool(&asset_memory_pool);

  log_info("Bye!");

  return 0;
}
