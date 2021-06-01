#pragma once

#include "types.hpp"

struct WindowSize {
  int32 width; // in pixels (size of framebuffer)
  int32 height; // in pixels (size of framebuffer)
  uint32 screencoord_width; // in screen coordinates
  uint32 screencoord_height; // in screen coordinates
};

#include "anim.hpp"
#include "gui.hpp"
#include "cameras.hpp"
#include "debugdraw.hpp"
#include "tasks.hpp"
#include "anim.hpp"
#include "memory.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "array.hpp"
#include "stackarray.hpp"
#include "queue.hpp"

namespace state {
  struct State {
    GLFWwindow *window;
    WindowSize window_size;
    EngineState engine_state;
    RendererState renderer_state;
    CamerasState cameras_state;
    InputState input_state;
    GuiState gui_state;
    DebugDrawState debug_draw_state;
    LightsState lights_state;
    AnimState anim_state;
    MaterialsState materials_state;
    TasksState tasks_state;
  };

  struct MemoryAndState {
    MemoryPool *asset_memory_pool;
    State *state;
  };
}

using state::State, state::MemoryAndState;
