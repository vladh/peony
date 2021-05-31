#include <thread>
#include "../src_external/pstr.h"
#include "types.hpp"
#include "logs.hpp"
#include "util.hpp"
#include "renderer.hpp"
#include "engine.hpp"
#include "state.hpp"
#include "peony_parser.hpp"
#include "intrinsics.hpp"


int main() {
  // Seed RNG
  srand((uint32)time(NULL));

  // Create memory pools
  MemoryPool state_memory_pool = {.size = sizeof(State)};
  defer { memory::destroy_memory_pool(&state_memory_pool); };
  MemoryPool asset_memory_pool = {.size = util::mb_to_b(1024)};
  defer { memory::destroy_memory_pool(&asset_memory_pool); };

  WindowSize window_size;
  GLFWwindow *window = renderer::init_window(&window_size);
  defer { renderer::destroy_window(); };
  if (!window) { return EXIT_FAILURE; }

  State *state = MEMORY_PUSH(&state_memory_pool, State, "state");
  state::init_state(state, &asset_memory_pool, window, &window_size);
  MemoryAndState memory_and_state = {&asset_memory_pool, state};
  glfwSetWindowUserPointer(window, &memory_and_state);

  std::mutex loading_thread_mutex;
  std::thread loading_threads[5];
  range (0, 5) {
    loading_threads[idx] = std::thread(
      tasks::run_loading_loop,
      &loading_thread_mutex,
      &state->should_stop,
      &state->task_queue,
      idx
    );
  }
  defer { range (0, 5) { loading_threads[idx].join(); } };

  renderer::init(
    &asset_memory_pool, &state->builtin_textures,
    window_size.width, window_size.height,
    state
  );
  engine::init(state);
  materials::init(
    &state->persistent_pbo, &state->texture_name_pool, &asset_memory_pool
  );
  gui::init(
    &state->gui_state,
    &asset_memory_pool,
    &state->input_state,
    window_size.width, window_size.height
  );
  debugdraw::init(&state->debug_draw_state, &asset_memory_pool);
  cameras::init(
    &state->camera_main,
    CameraType::perspective,
    window_size.width,
    window_size.height
  );
  state->camera_active = &state->camera_main;
  input::init(&state->input_state, window);

  engine::run_main_loop(state);

  return EXIT_SUCCESS;
}
