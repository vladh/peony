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
  srand((uint32)time(NULL));
  defer { logs::info("Bye!"); };

  MemoryPool state_memory_pool = {.size = sizeof(State)};
  defer { memory::destroy_memory_pool(&state_memory_pool); };
  MemoryPool asset_memory_pool = {.size = util::mb_to_b(1024)};
  defer { memory::destroy_memory_pool(&asset_memory_pool); };

  WindowInfo window_info = renderer::init_window();
  defer { renderer::destroy_window(); };
  if (!window_info.window) { return EXIT_FAILURE; }

  State *state = MEMORY_PUSH(&state_memory_pool, State, "state");
  state::init_state(state, &asset_memory_pool, window_info);
  MemoryAndState memory_and_state = {&asset_memory_pool, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

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
    window_info.width, window_info.height,
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
    state->window_info.width, state->window_info.height
  );
  debugdraw::init(&state->debug_draw_state, &asset_memory_pool);
  cameras::init(
    &state->camera_main,
    CameraType::perspective,
    state->window_info.width,
    state->window_info.height
  );
  state->camera_active = &state->camera_main;
  input::init(
    &state->input_state,
    state->window_info.window
  );

  engine::run_main_loop(state);

  return EXIT_SUCCESS;
}
