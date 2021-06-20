/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include "types.hpp"
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
  EngineState engine_state;
  RendererState renderer_state;
  CamerasState cameras_state;
  InputState input_state;
  GuiState gui_state;
  LightsState lights_state;
  AnimState anim_state;
  MaterialsState materials_state;
  TasksState tasks_state;
  DebugDrawState debug_draw_state;
  BehaviorState behavior_state;
};

struct MemoryAndState {
  MemoryPool *asset_memory_pool;
  State *state;
};
