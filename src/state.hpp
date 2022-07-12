// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "common.hpp"
#include "anim.hpp"
#include "gui.hpp"
#include "cameras.hpp"
#include "debugdraw.hpp"
#include "tasks.hpp"
#include "anim.hpp"
#include "memory.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "behavior.hpp"
#include "array.hpp"
#include "stackarray.hpp"
#include "queue.hpp"
#include "core.hpp"

struct State {
    GLFWwindow *window;
    WindowSize window_size;
    engine::State engine_state;
    renderer::State renderer_state;
    cameras::State cameras_state;
    input::State input_state;
    gui::State gui_state;
    lights::State lights_state;
    anim::State anim_state;
    mats::State materials_state;
    tasks::State tasks_state;
    debugdraw::State debug_draw_state;
    behavior::State behavior_state;
    memory::Pool *asset_memory_pool;
};

struct MemoryAndState {
    memory::Pool *asset_memory_pool;
    State *state;
};
