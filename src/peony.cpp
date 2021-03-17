#define _CRT_SECURE_NO_WARNINGS

#define USE_TIMERS true
#define USE_VLD false
#define USE_MEMORY_DEBUG_LOGS false
#define USE_MEMORYPOOL_ITEM_DEBUG false
#define USE_CACHELINE_SIZE_DISPLAY false
#define USE_FULLSCREEN true
#define USE_WINDOWED_FULLSCREEN true
#define TARGET_MONITOR 0

#include "peony.hpp"

#include "log.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "tasks.cpp"
#include "peony_file_parser.cpp"
#include "materials.cpp"
#include "fonts.cpp"
#include "shaders.cpp"
#include "camera.cpp"
#include "memory.cpp"
#include "input.cpp"
#include "entities.cpp"
#include "entity_sets.cpp"
#include "gui.cpp"
#include "models.cpp"
#include "world.cpp"
#include "state.cpp"
#include "renderer.cpp"


void process_input(GLFWwindow *window, State *state) {
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

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_TAB)) {
    state->should_pause = !state->should_pause;
  }

  if (Input::is_key_now_down(&state->input_state, GLFW_KEY_BACKSPACE)) {
    state->should_hide_ui = !state->should_hide_ui;
  }

  if (Input::is_key_down(&state->input_state, GLFW_KEY_ENTER)) {
    state->should_manually_advance_to_next_frame = true;
  }
}


void run_main_loop(State *state) {
  std::chrono::steady_clock::time_point second_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point frame_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point last_frame_start = std::chrono::steady_clock::now();
  // 1/165 seconds (for 165 fps)
  std::chrono::nanoseconds frame_duration = std::chrono::nanoseconds(6060606);
  std::chrono::steady_clock::time_point time_frame_should_end;
  uint32 n_frames_this_second = 0;
  state->n_frames_since_start = 0;

  while (!state->should_stop) {
    glfwPollEvents();
    process_input(state->window_info.window, state);

    if (
      !state->is_manual_frame_advance_enabled ||
      state->should_manually_advance_to_next_frame
    ) {
      state->n_frames_since_start++;
      last_frame_start = frame_start;
      frame_start = std::chrono::steady_clock::now();
      time_frame_should_end = frame_start + frame_duration;

      // If we should pause, stop time-based events.
      if (!state->should_pause) {
        state->dt = std::chrono::duration_cast<std::chrono::duration<real64>>(
          frame_start - last_frame_start
        ).count();

        state->dt_hist[state->dt_hist_idx] = state->dt;
        state->dt_hist_idx++;
        if (state->dt_hist_idx >= DT_HIST_LENGTH) {
          state->dt_hist_idx = 0;
        }
        real64 dt_hist_sum = 0.0f;
        for (uint32 idx = 0; idx < DT_HIST_LENGTH; idx++) {
          dt_hist_sum += state->dt_hist[idx];
        }
        state->dt_average = dt_hist_sum / DT_HIST_LENGTH;

        state->t += state->dt;
      }

      // Count FPS.
      n_frames_this_second++;
      std::chrono::duration<real64> time_since_second_start = frame_start - second_start;
      if (time_since_second_start >= std::chrono::seconds(1)) {
        second_start = frame_start;
        state->last_fps = n_frames_this_second;
        n_frames_this_second = 0;
        if (state->should_hide_ui) {
          log_info("%.2f ms", state->dt_average * 1000.0f);
        }
      }

      // NOTE: Don't render on the very first frame. This avoids flashing that happens in
      // fullscreen. There is a better way to handle this, but whatever, figure it out later.
      if (state->n_frames_since_start > 1) {
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


/* int WinMain( */
/*   HINSTANCE hInstance, */
/*   HINSTANCE hPrevInstance, */
/*   LPSTR lpCmdLine, */
/*   int nShowCmd */
/* ) { */
int main() {
  check_environment();
  srand((uint32)time(NULL));

  START_TIMER(init);

  MemoryPool state_memory_pool = {};
  state_memory_pool.size = sizeof(State);
  MemoryPool asset_memory_pool = {};
  MemoryPool entity_memory_pool = {};

  WindowInfo window_info;
  START_TIMER(init_window);
  Renderer::init_window(&window_info);
  END_TIMER(init_window);
  if (!window_info.window) {
    return -1;
  }

  State *state = init_state(
    (State*)Memory::push(
      &state_memory_pool, sizeof(State), "state"
    ),
    &asset_memory_pool,
    &entity_memory_pool,
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

#if 0
  Util::print_texture_internalformat_info(GL_RGB8);
  Util::print_texture_internalformat_info(GL_RGBA8);
  Util::print_texture_internalformat_info(GL_SRGB8);
#endif

  Renderer::update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&asset_memory_pool, &entity_memory_pool, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  Materials::init_texture_name_pool(&state->texture_name_pool, &asset_memory_pool, 64, 4);
  Renderer::init_g_buffer(&asset_memory_pool, state);
  Renderer::init_l_buffer(&asset_memory_pool, state);
  Renderer::init_blur_buffers(&asset_memory_pool, state);
  Renderer::init_shadowmaps(&asset_memory_pool, state);
  Renderer::init_ubo(state);
  World::init(&asset_memory_pool, &entity_memory_pool, state);

  Materials::init_persistent_pbo(&state->persistent_pbo, 25, 2048, 2048, 4);

  Gui::init_gui_state(
    &state->gui_state,
    &asset_memory_pool,
    &state->shader_assets,
    &state->input_state,
    state->window_info.width, state->window_info.height
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

#if USE_CACHELINE_SIZE_DISPLAY
  log_info("Cache line size: %dB", cacheline_get_size());
#endif

  END_TIMER(init);

  run_main_loop(state);

  loading_thread_1.join();
  loading_thread_2.join();
  loading_thread_3.join();
  loading_thread_4.join();
  loading_thread_5.join();

  Renderer::destroy_window();
  Memory::destroy_memory_pool(&state_memory_pool);
  Memory::destroy_memory_pool(&asset_memory_pool);
  Memory::destroy_memory_pool(&entity_memory_pool);

  log_info("Bye!");

  return 0;
}
