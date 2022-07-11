// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "engine.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "mats.hpp"
#include "input.hpp"

namespace debug_ui {
  void render_debug_ui(
    EngineState *engine_state,
    mats::State *materials_state,
    InputState *input_state,
    renderer::WindowSize *window_size
  );
}
