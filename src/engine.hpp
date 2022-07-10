// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"
#include "lights.hpp"
#include "behavior.hpp"
#include "physics.hpp"
#include "models.hpp"
#include "renderer.hpp"
#include "mats.hpp"
#include "core.hpp"
#include "cameras.hpp"

static constexpr uint32 DT_HIST_LENGTH = 512;

struct TimingInfo {
  chrono::steady_clock::time_point frame_start;
  chrono::steady_clock::time_point last_frame_start;
  chrono::nanoseconds frame_duration;
  chrono::steady_clock::time_point time_frame_should_end;

  chrono::steady_clock::time_point second_start;
  uint32 n_frames_this_second;
  uint32 n_frames_since_start;
};

struct PerfCounters {
  real64 dt_average;
  real64 dt_hist[DT_HIST_LENGTH];
  uint32 dt_hist_idx;
  uint32 last_fps;
};

struct EngineState {
  bool32 is_manual_frame_advance_enabled;
  bool32 should_manually_advance_to_next_frame;
  bool32 should_stop;
  bool32 should_pause;
  bool32 should_limit_fps;
  char current_scene_name[MAX_COMMON_NAME_LENGTH];
  // NOTE: `t` and `dt` will not change when gameplay is paused.
  real64 t; // us
  real64 dt; // us
  real64 timescale_diff;
  PerfCounters perf_counters;
  uint32 n_valid_model_loaders;
  uint32 n_valid_entity_loaders;
  bool32 is_world_loaded;
  bool32 was_world_ever_loaded;
  uint32 first_non_internal_material_idx;
  Array<models::ModelLoader> model_loaders;
  models::EntityLoaderSet entity_loader_set;
  EntitySet entity_set;
  drawable::ComponentSet drawable_component_set;
  LightComponentSet light_component_set;
  SpatialComponentSet spatial_component_set;
  BehaviorComponentSet behavior_component_set;
  AnimationComponentSet animation_component_set;
  physics::ComponentSet physics_component_set;
};

namespace engine {
  extern real64 *g_t;
  extern real64 *g_dt;

  void run_main_loop(
    EngineState *engine_state,
    renderer::State *renderer_state,
    mats::State *materials_state,
    CamerasState *cameras_state,
    GuiState *gui_state,
    InputState *input_state,
    LightsState *lights_state,
    TasksState *tasks_state,
    AnimState *anim_state,
    BehaviorState *behavior_state,
    GLFWwindow *window,
    renderer::WindowSize *window_size
  );
  void init(EngineState *engine_state, MemoryPool *asset_memory_pool);
}
