#define _CRT_SECURE_NO_WARNINGS

#define USE_TIMERS true
#define USE_VLD false
#define USE_MEMORY_DEBUG_LOGS false
#define USE_MEMORYPOOL_ITEM_DEBUG false
#define USE_CACHELINE_SIZE_DISPLAY false
#define USE_FULLSCREEN false

#include "peony.hpp"

global_variable uint32 global_oopses = 0;

#include "log.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "task.cpp"
#include "peony_file_parser.cpp"
#include "textures.cpp"
#include "font_asset.cpp"
#include "shader_asset.cpp"
#include "camera.cpp"
#include "memory_pool.cpp"
#include "memory.cpp"
#include "input_manager.cpp"
#include "entity_manager.cpp"
#include "behavior_component.cpp"
#include "drawable_component.cpp"
#include "light_component.cpp"
#include "spatial_component.cpp"
#include "behavior_component_manager.cpp"
#include "drawable_component_manager.cpp"
#include "light_component_manager.cpp"
#include "spatial_component_manager.cpp"
#include "gui_manager.cpp"
#include "model_asset.cpp"
#include "world.cpp"
#include "state.cpp"
#include "renderer.cpp"


void update_light_position(State *state, real32 amount) {
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


void process_input(GLFWwindow *window, State *state, Memory *memory) {
  // Continuous
  if (state->input_manager.is_key_down(GLFW_KEY_W)) {
    state->camera_active->move_front_back(1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_S)) {
    state->camera_active->move_front_back(-1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_A)) {
    state->camera_active->move_left_right(-1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_D)) {
    state->camera_active->move_left_right(1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_Z)) {
    update_light_position(state, 0.10f * (real32)state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_X)) {
    update_light_position(state, -0.10f * (real32)state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_SPACE)) {
    state->camera_active->move_up_down(1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_LEFT_CONTROL)) {
    state->camera_active->move_up_down(-1, state->dt);
  }

  // Transient
  if (state->input_manager.is_key_now_down(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (state->input_manager.is_key_now_down(GLFW_KEY_C)) {
    state->is_cursor_disabled = !state->is_cursor_disabled;
    Renderer::update_drawing_options(state, window);
  }

  if (state->input_manager.is_key_now_down(GLFW_KEY_TAB)) {
    state->should_pause = !state->should_pause;
  }

  if (state->input_manager.is_key_now_down(GLFW_KEY_BACKSPACE)) {
    state->should_hide_ui = !state->should_hide_ui;
  }

  if (state->input_manager.is_key_down(GLFW_KEY_ENTER)) {
    state->should_manually_advance_to_next_frame = true;
  }
}


void run_main_loop(Memory *memory, State *state) {
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
    process_input(state->window_info.window, state, memory);

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
        World::update(memory, state);
        Renderer::render(memory, state);
      }
      if (state->is_manual_frame_advance_enabled) {
        state->should_manually_advance_to_next_frame = false;
      }
      state->input_manager.reset_n_mouse_button_state_changes_this_frame();
      state->input_manager.reset_n_key_state_changes_this_frame();

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


void destroy_window() {
  glfwTerminate();
}


void run_loading_loop(
  std::mutex *mutex, Memory *memory, State *state, uint32 idx_thread
) {
  while (!state->should_stop) {
    Task *task = nullptr;

    mutex->lock();
    if (state->task_queue.size > 0) {
      task = state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      log_info(
        "[Thread #%d] Running task %s for model %s",
        idx_thread,
        Task::task_type_to_str(task->type),
        task->model_asset->name
      );
      task->run();
      log_info(
        "[Thread #%d] Finished task %s for model %s",
        idx_thread,
        Task::task_type_to_str(task->type),
        task->model_asset->name
      );
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}


void check_environment() {
  // Check that an `enum class`'s default value == its first element == 0;
  BehaviorComponent test;
  assert(test.behavior == Behavior::none);
}


int main() {
  check_environment();

  START_TIMER(init);

  srand((uint32)time(NULL));
  START_TIMER(allocate_memory);
  Memory memory;
  END_TIMER(allocate_memory);

  WindowInfo window_info;
  START_TIMER(init_window);
  Renderer::init_window(&window_info);
  END_TIMER(init_window);
  if (!window_info.window) {
    return -1;
  }

  State *state = new((State*)memory.state_memory) State(&memory, window_info);

  ModelAsset::entity_manager = &state->entity_manager;
  ModelAsset::drawable_component_manager = &state->drawable_component_manager;
  ModelAsset::spatial_component_manager = &state->spatial_component_manager;
  ModelAsset::light_component_manager = &state->light_component_manager;
  ModelAsset::behavior_component_manager = &state->behavior_component_manager;

  std::mutex loading_thread_mutex;
  std::thread loading_thread_1 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 1);
  std::thread loading_thread_2 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 2);
  std::thread loading_thread_3 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 3);
  std::thread loading_thread_4 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 4);
  std::thread loading_thread_5 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 5);

#if 0
  Util::print_texture_internalformat_info(GL_RGB8);
  Util::print_texture_internalformat_info(GL_RGBA8);
  Util::print_texture_internalformat_info(GL_SRGB8);
#endif

  Renderer::update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&memory, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  Textures::init_texture_name_pool(&state->texture_name_pool, &memory, 64, 4);
  Renderer::init_g_buffer(&memory, state);
  Renderer::init_l_buffer(&memory, state);
  Renderer::init_blur_buffers(&memory, state);
  Renderer::init_shadowmaps(&memory, state);
  Renderer::init_ubo(&memory, state);
  World::init(&memory, state);
  Textures::init_persistent_pbo(&state->persistent_pbo, 25, 2048, 2048, 4);

#if 0
  memory.asset_memory_pool.print();
  memory.entity_memory_pool.print();
  memory.temp_memory_pool.print();
#endif

#if USE_CACHELINE_SIZE_DISPLAY
  log_info("Cache line size: %dB", cacheline_get_size());
#endif

  END_TIMER(init);

  run_main_loop(&memory, state);

  loading_thread_1.join();
  loading_thread_2.join();
  loading_thread_3.join();
  loading_thread_4.join();
  loading_thread_5.join();

  destroy_window();

  log_info("Bye!");

  return 0;
}
