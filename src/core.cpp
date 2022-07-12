// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include <thread>
#include "../src_external/pstr.h"
#include "types.hpp"
#include "logs.hpp"
#include "util.hpp"
#include "renderer.hpp"
#include "internals.hpp"
#include "engine.hpp"
#include "state.hpp"
#include "peony_parser.hpp"
#include "constants.hpp"
#include "core.hpp"
#include "intrinsics.hpp"


State *core::state = nullptr;


WindowSize *
core::get_window_size()
{
    return &core::state->window_size;
}


memory::Pool *
core::get_asset_memory_pool()
{
    return core::state->asset_memory_pool;
}


int
core::run()
{
    // Allocate memory
    State *state = (State*)calloc(1, sizeof(State));
    defer { free(state); };
    memory::Pool asset_memory_pool = { .size = util::mb_to_b(1024) };
    defer { memory::destroy_memory_pool(&asset_memory_pool); };

    // Make state
    if (!init_state(state, &asset_memory_pool)) {
        return EXIT_FAILURE;
    }
    defer { destroy_state(state); };

    // Set up loading threads
    std::mutex loading_thread_mutex;
    std::thread loading_threads[N_LOADING_THREADS];
    range (0, N_LOADING_THREADS) {
        loading_threads[idx] = std::thread(
            tasks::run_loading_loop,
            &loading_thread_mutex,
            &state->engine_state.should_stop,
            idx);
    }
    defer { range (0, N_LOADING_THREADS) { loading_threads[idx].join(); } };

    // Run main loop
    engine::run_main_loop(state->window);

    return EXIT_SUCCESS;
}


void
core::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    memory::Pool *asset_memory_pool = get_asset_memory_pool();
    WindowSize *window_size = get_window_size();
    logs::info("Window is now: %d x %d", window_size->width, window_size->height);
    window_size->width = width;
    window_size->height = height;
    cameras::Camera *camera = cameras::get_main();
    cameras::update_matrices(camera);
    cameras::update_ui_matrices(camera);
    gui::update_screen_dimensions(window_size->width, window_size->height);

    auto *builtin_textures = renderer::get_builtin_textures();
    renderer::resize_renderer_buffers(asset_memory_pool, builtin_textures,
        width, height);
}


void
core::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    input::update_mouse_button(button, action, mods);
    gui::update_mouse_button();
}


void
core::mouse_callback(GLFWwindow *window, f64 x, f64 y)
{
    v2 mouse_pos = v2(x, y);
    input::update_mouse(mouse_pos);

    if (input::is_cursor_enabled()) {
        gui::update_mouse();
    } else {
        cameras::Camera *camera = cameras::get_main();
        cameras::update_mouse(camera, input::get_mouse_3d_offset());
    }
}


void
core::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    input::update_keys(key, scancode, action, mods);
}


void
core::char_callback(GLFWwindow* window, u32 codepoint) {
    input::update_text_input(codepoint);
}


bool
core::init_state(State *state, memory::Pool *asset_memory_pool)
{
    core::state = state;
    state->window = renderer::init_window(&state->window_size);
    if (!state->window) { return false; }

    state->asset_memory_pool = asset_memory_pool;

    engine::init(&state->engine_state, asset_memory_pool);
    mats::init(&state->materials_state, asset_memory_pool);
    input::init(&state->input_state, state->window);
    renderer::init(
        &state->renderer_state, asset_memory_pool,
        state->window_size.width, state->window_size.height, state->window);
    internals::init();
    gui::init(asset_memory_pool,
        &state->gui_state,
        renderer::get_gui_texture_atlas_size(),
        renderer::get_gui_font_assets(),
        state->window_size.width, state->window_size.height);
    debugdraw::init(&state->debug_draw_state, asset_memory_pool);
    lights::init(&state->lights_state);
    tasks::init(&state->tasks_state, asset_memory_pool);
    anim::init(&state->anim_state, asset_memory_pool);
    cameras::init(&state->cameras_state, state->window_size.width, state->window_size.height);
    behavior::init(
        &state->behavior_state,
        // NOTE: behavior needs the global state to pass to the behavior functions
        state);

    return true;
}


void
core::destroy_state(State *state)
{
    glfwTerminate();
}
