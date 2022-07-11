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


int
core::run()
{
    // Allocate memory
    State *state = (State*)calloc(1, sizeof(State));
    defer { free(state); };
    MemoryPool asset_memory_pool = { .size = util::mb_to_b(1024) };
    defer { memory::destroy_memory_pool(&asset_memory_pool); };

    // Make state
    if (!init_state(state, &asset_memory_pool)) {
        return EXIT_FAILURE;
    }
    defer { destroy_state(state); };

    // Set up globals
    MemoryAndState memory_and_state = { &asset_memory_pool, state };
    glfwSetWindowUserPointer(state->window, &memory_and_state);

    // Set up loading threads
    std::mutex loading_thread_mutex;
    std::thread loading_threads[N_LOADING_THREADS];
    range (0, N_LOADING_THREADS) {
        loading_threads[idx] = std::thread(
            tasks::run_loading_loop,
            &state->tasks_state,
            &loading_thread_mutex,
            &state->engine_state.should_stop,
            idx
        );
    }
    defer { range (0, N_LOADING_THREADS) { loading_threads[idx].join(); } };

    // Run main loop
    engine::run_main_loop(
        &state->engine_state,
        &state->materials_state,
        &state->cameras_state,
        &state->input_state,
        &state->lights_state,
        &state->tasks_state,
        &state->anim_state,
        &state->behavior_state,
        state->window,
        &state->window_size);

    return EXIT_SUCCESS;
}


void
core::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    MemoryPool *asset_memory_pool = memory_and_state->asset_memory_pool;
    logs::info("Window is now: %d x %d", state->window_size.width, state->window_size.height);
    state->window_size.width = width;
    state->window_size.height = height;
    cameras::update_matrices(
        state->cameras_state.camera_active, state->window_size.width, state->window_size.height);
    cameras::update_ui_matrices(
        state->cameras_state.camera_active, state->window_size.width, state->window_size.height);
    gui::update_screen_dimensions(state->window_size.width, state->window_size.height);

    auto *builtin_textures = renderer::get_builtin_textures();
    renderer::resize_renderer_buffers(
        asset_memory_pool, &state->materials_state.materials, builtin_textures,
        width, height);
}


void
core::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;

    input::update_mouse_button(&state->input_state, button, action, mods);
    gui::update_mouse_button();
}


void
core::mouse_callback(GLFWwindow *window, real64 x, real64 y)
{
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;

    v2 mouse_pos = v2(x, y);
    input::update_mouse(&state->input_state, mouse_pos);

    if (state->input_state.is_cursor_enabled) {
        gui::update_mouse();
    } else {
        cameras::update_mouse(state->cameras_state.camera_active, state->input_state.mouse_3d_offset);
    }
}


void
core::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    input::update_keys(&state->input_state, key, scancode, action, mods);
}


void
core::char_callback(GLFWwindow* window, uint32 codepoint) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    input::update_text_input(&state->input_state, codepoint);
}


bool
core::init_state(State *state, MemoryPool *asset_memory_pool)
{
    state->window = renderer::init_window(&state->window_size);
    if (!state->window) { return false; }

    engine::init(&state->engine_state, asset_memory_pool);
    mats::init(&state->materials_state, asset_memory_pool);
    renderer::init(
        &state->renderer_state, &state->input_state, asset_memory_pool,
        state->window_size.width, state->window_size.height, state->window);
    internals::init(&state->engine_state, &state->materials_state);
    gui::init(asset_memory_pool,
        &state->gui_state,
        &state->input_state,
        renderer::get_gui_texture_atlas_size(),
        renderer::get_gui_font_assets(),
        state->window_size.width, state->window_size.height);
    debugdraw::init(&state->debug_draw_state, asset_memory_pool);
    input::init(&state->input_state, state->window);
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
