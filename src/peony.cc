#include "peony.hpp"

#include "types.cpp"
#include "constants.cpp"
#include "intrinsics.cpp"
#include "debug.cpp"
#include "logs.cpp"
#include "util.cpp"
#include "memory.cpp"
#include "files.cpp"
#include "str.cpp"
#include "pack.cpp"
#include "queue.cpp"
#include "array.cpp"
#include "stackarray.cpp"
#include "shaders.cpp"
#include "tasks.cpp"
#include "materials.cpp"
#include "fonts.cpp"
#include "entities.cpp"
#include "spatial.cpp"
#include "lights.cpp"
#include "behavior.cpp"
#include "anim.cpp"
#include "debugdraw.cpp"
#include "input.cpp"
#include "gui.cpp"
#include "physics.cpp"
#include "models.cpp"
#include "peonyparser.cpp"
#include "camera.cpp"
#include "state.cpp"
#include "renderer.cpp"
#include "engine.cpp"
#include "behavior_functions.cpp"


int main() {
  // TODO: Change
  behavior::function_map[0] = (BehaviorFunction)nullptr;
  behavior::function_map[1] = (BehaviorFunction)behavior_functions::test;
  behavior::function_map[2] = (BehaviorFunction)behavior_functions::char_movement_test;

  srand((uint32)time(NULL));
  defer { logs::info("Bye!"); };

  MemoryPool state_memory_pool = {.size = sizeof(State)};
  defer { memory::destroy_memory_pool(&state_memory_pool); };
  MemoryPool asset_memory_pool = {.size = util::mb_to_b(1024)};
  defer { memory::destroy_memory_pool(&asset_memory_pool); };

  WindowInfo window_info = renderer::init_window();
  defer { renderer::destroy_window(); };
  if (!window_info.window) { return EXIT_FAILURE; }

  State *state = (State*)memory::push(&state_memory_pool, sizeof(State), "state");
  engine::init_state(state, &asset_memory_pool, window_info);

  std::mutex loading_thread_mutex;
  std::thread loading_threads[5];
  for_range (0, 5) {
    loading_threads[idx] = std::thread(
      tasks::run_loading_loop,
      &loading_thread_mutex,
      &state->should_stop,
      &state->task_queue,
      idx
    );
  }
  defer {
    for_range (0, 5) {
      loading_threads[idx].join();
    }
  };

  renderer::update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&asset_memory_pool, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  materials::init_texture_name_pool(
    &state->texture_name_pool, &asset_memory_pool, 64, 4
  );
  renderer::init_g_buffer(
    &asset_memory_pool, &state->builtin_textures, window_info.width, window_info.height
  );
  renderer::init_l_buffer(
    &asset_memory_pool, &state->builtin_textures, window_info.width, window_info.height
  );
  renderer::init_blur_buffers(
    &asset_memory_pool, &state->builtin_textures, window_info.width, window_info.height
  );
  renderer::init_shadowmaps(&asset_memory_pool, &state->builtin_textures);
  renderer::init_ubo(state);
  engine::init(state);

  materials::init_persistent_pbo(&state->persistent_pbo, 25, 2048, 2048, 4);

  gui::init_gui_state(
    &state->gui_state,
    &asset_memory_pool,
    &state->input_state,
    state->window_info.width, state->window_info.height
  );
  gui::log("Hello world!");

  debugdraw::init_debug_draw_state(
    &state->debug_draw_state,
    &asset_memory_pool
  );

  cameras::init_camera(
    &state->camera_main,
    CameraType::perspective,
    state->window_info.width,
    state->window_info.height
  );
  state->camera_active = &state->camera_main;

  input::init_input_state(
    &state->input_state,
    state->window_info.window
  );

  engine::run_main_loop(state);

  return EXIT_SUCCESS;
}
