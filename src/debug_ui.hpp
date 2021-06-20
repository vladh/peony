/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include "engine.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "materials.hpp"
#include "input.hpp"

namespace debug_ui {
  void render_debug_ui(
    EngineState *engine_state,
    RendererState *renderer_state,
    GuiState *gui_state,
    MaterialsState *materials_state,
    InputState *input_state,
    WindowSize *window_size
  );
}
