// All our headers
#include "peony.hpp"

// Global variables
global real64 g_t;
global GameConsole *g_console;
global DebugDrawState *g_dds;

// We're doing a unity build, so all our code goes here
#include "logs.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "str.cpp"
#include "tasks.cpp"
#include "peonyparser.cpp"
#include "materials.cpp"
#include "fonts.cpp"
#include "shaders.cpp"
#include "camera.cpp"
#include "memory.cpp"
#include "input.cpp"
#include "physics.cpp"
#include "entities.cpp"
#include "behavior_functions.cpp"
#include "entitysets.cpp"
#include "gui.cpp"
#include "debugdraw.cpp"
#include "models.cpp"
#include "world.cpp"
#include "renderer.cpp"
#include "engine.cpp"


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

  State *state = (State*)memory::push(&state_memory_pool, sizeof(State), "state");
  engine::init_state(state, &asset_memory_pool, window_info);

  std::mutex loading_thread_mutex;
  std::thread loading_threads[5] = {
    std::thread(tasks::run_loading_loop, &loading_thread_mutex, state, 1),
    std::thread(tasks::run_loading_loop, &loading_thread_mutex, state, 2),
    std::thread(tasks::run_loading_loop, &loading_thread_mutex, state, 3),
    std::thread(tasks::run_loading_loop, &loading_thread_mutex, state, 4),
    std::thread(tasks::run_loading_loop, &loading_thread_mutex, state, 5),
  };
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
  world::init(state);

  materials::init_persistent_pbo(&state->persistent_pbo, 25, 2048, 2048, 4);

  gui::init_gui_state(
    &state->gui_state,
    &asset_memory_pool,
    &state->input_state,
    state->window_info.width, state->window_info.height
  );
  logs::console("Hello world!");

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
